#include "snmp_sensor.h"

namespace esphome {
namespace snmp {

static const char *const TAG = "snmp";

// Přidejte inicializaci do konstruktoru (pokud je v .cpp)
// Nebo přesunout konstruktor do .cpp pokud je potřeba

void SnmpSensor::setup() {
  if (udp_.begin(0)) {
    initialized_ = true;
    ESP_LOGI(TAG, "SNMP sensor initialized for OID: %s", this->oid_.c_str());
  } else {
    ESP_LOGE(TAG, "Failed to initialize UDP for SNMP");
  }
}

void SnmpSensor::update() {
  if (!initialized_) {
    return;
  }
  
  // Reset state if we're stuck
  if (waiting_response_ && (millis() - send_time_ > timeout_ * 2)) {
    ESP_LOGW(TAG, "Reset stuck SNMP query");
    waiting_response_ = false;
    udp_.stop();
    udp_.begin(0);
  }
  
  // Only send new query if not waiting for response
  if (!waiting_response_) {
    this->send_snmp_get();
  }
}

void SnmpSensor::loop() {
  if (!initialized_ || !waiting_response_) {
    return;
  }
  
  // Check timeout
  if (millis() - send_time_ > timeout_) {
    ESP_LOGW(TAG, "SNMP timeout for OID: %s", this->oid_.c_str());
    waiting_response_ = false;
    this->publish_state(NAN);
    return;
  }
  
  this->handle_response();
}

// Zbytek implementace zůstává stejný...
void SnmpSensor::send_snmp_get() {
  IPAddress ip;
  if (!ip.fromString(this->host_.c_str())) {
    ESP_LOGE(TAG, "Invalid IP address: %s", this->host_.c_str());
    return;
  }

  // Simple SNMP GET request (SNMPv1)
  std::vector<uint8_t> packet;
  
  // SNMP Message Sequence
  packet.insert(packet.end(), {0x30, 0x29});
  
  // Version: INTEGER 0
  packet.insert(packet.end(), {0x02, 0x01, 0x00});
  
  // Community: OCTET STRING
  packet.push_back(0x04);
  packet.push_back(this->community_.length());
  packet.insert(packet.end(), this->community_.begin(), this->community_.end());
  
  // PDU: Get Request
  packet.insert(packet.end(), {0xA0, 0x1C});
  
  // Request ID
  packet.insert(packet.end(), {0x02, 0x04, 0x00, 0x00, 0x00, 0x01});
  
  // Error status
  packet.insert(packet.end(), {0x02, 0x01, 0x00});
  
  // Error index
  packet.insert(packet.end(), {0x02, 0x01, 0x00});
  
  // Variable bindings
  packet.insert(packet.end(), {0x30, 0x0E});
  
  // Single variable binding
  packet.insert(packet.end(), {0x30, 0x0C});
  
  // OID - simplified for APC UPS
  std::string oid_bytes = this->oid_to_bytes(this->oid_);
  packet.push_back(0x06);
  packet.push_back(oid_bytes.length());
  packet.insert(packet.end(), oid_bytes.begin(), oid_bytes.end());
  
  // Value (null)
  packet.insert(packet.end(), {0x05, 0x00});
  
  // Send packet - with non-blocking checks
  if (this->udp_.beginPacket(this->host_.c_str(), this->port_)) {
    size_t written = this->udp_.write(packet.data(), packet.size());
    if (written == packet.size()) {
      if (this->udp_.endPacket()) {
        this->waiting_response_ = true;
        this->send_time_ = millis();
        ESP_LOGD(TAG, "SNMP query sent to %s for OID: %s", this->host_.c_str(), this->oid_.c_str());
      } else {
        ESP_LOGE(TAG, "Failed to end SNMP packet");
      }
    } else {
      ESP_LOGE(TAG, "Failed to write SNMP packet: %d/%d bytes", written, packet.size());
    }
  } else {
    ESP_LOGE(TAG, "Failed to begin SNMP packet");
  }
}

void SnmpSensor::handle_response() {
  int packet_size = this->udp_.parsePacket();
  if (packet_size > 0) {
    uint8_t buffer[256];
    int len = this->udp_.read(buffer, sizeof(buffer));
    
    if (len > 0) {
      float value = this->parse_snmp_response(buffer, len);
      if (!isnan(value)) {
        this->publish_state(value);
        ESP_LOGD(TAG, "SNMP value received: %.2f", value);
      } else {
        ESP_LOGW(TAG, "Failed to parse SNMP response");
        this->publish_state(NAN);
      }
    } else {
      ESP_LOGW(TAG, "Empty SNMP response");
      this->publish_state(NAN);
    }
    this->waiting_response_ = false;
  }
}

std::string SnmpSensor::oid_to_bytes(const std::string &oid) {
  // Simplified OID conversion for common APC OIDs
  if (oid == "1.3.6.1.4.1.318.1.1.1.3.2.1.0") {
    return std::string("\x2b\x06\x01\x04\x01\x4e\x01\x01\x01\x03\x02\x01\x00", 13);
  } else if (oid == "1.3.6.1.4.1.318.1.1.1.2.2.3.0") {
    return std::string("\x2b\x06\x01\x04\x01\x4e\x01\x01\x01\x02\x02\x03\x00", 13);
  } else if (oid == "1.3.6.1.4.1.318.1.1.1.2.2.1.0") {
    return std::string("\x2b\x06\x01\x04\x01\x4e\x01\x01\x01\x02\x02\x01\x00", 13);
  }
  
  ESP_LOGW(TAG, "Unknown OID, using fallback: %s", oid.c_str());
  std::vector<uint8_t> result;
  result.push_back(0x2b);
  result.push_back(0x06);
  result.push_back(0x01);
  result.push_back(0x04);
  result.push_back(0x01);
  result.push_back(0x4e);
  return std::string(result.begin(), result.end());
}

float SnmpSensor::parse_snmp_response(const uint8_t *buffer, size_t length) {
  // Look for INTEGER values in SNMP response
  for (size_t i = 0; i < length - 2; i++) {
    if (buffer[i] == 0x02) { // INTEGER type
      uint8_t len = buffer[i+1];
      if (len == 0x04 && i + 6 < length) { // 4-byte integer
        int32_t value = (buffer[i+2] << 24) | (buffer[i+3] << 16) | 
                       (buffer[i+4] << 8) | buffer[i+5];
        return static_cast<float>(value);
      } else if (len == 0x02 && i + 4 < length) { // 2-byte integer
        int32_t value = (buffer[i+2] << 8) | buffer[i+3];
        return static_cast<float>(value);
      } else if (len == 0x01 && i + 3 < length) { // 1-byte integer
        int32_t value = buffer[i+2];
        return static_cast<float>(value);
      }
    }
  }
  return NAN;
}

}  // namespace snmp
}  // namespace esphome
