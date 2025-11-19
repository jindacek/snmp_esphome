#include "snmp_sensor.h"

namespace esphome {
namespace snmp {

static const char *const TAG = "snmp";

void SnmpSensor::setup() {
  // Inicializace UDP socketu - PŘESUNUTO ZDE z update()
  if (udp_.begin(0)) {
    initialized_ = true;
    ESP_LOGI(TAG, "SNMP sensor initialized for OID: %s", this->oid_.c_str());
  } else {
    ESP_LOGE(TAG, "Failed to initialize UDP for SNMP");
  }
}

void SnmpSensor::update() {
  // Čekáme na WiFi
  if (!initialized_ || !network::is_connected()) {
    ESP_LOGW(TAG, "WiFi not connected, skipping SNMP update");
    return;
  }
  
  if (waiting_response_) {
    // Pokud čekáme na odpověď, nekontrolujeme timeout zde - to dělá loop()
    return;
  }
  
  // Odeslat nový dotaz pouze pokud nečekáme na odpověď
  this->send_snmp_get();
}

void SnmpSensor::loop() {
  if (!initialized_ || !waiting_response_) {
    return;
  }
  
  // Kontrola timeoutu
  if (millis() - send_time_ > timeout_) {
    ESP_LOGW(TAG, "SNMP timeout for OID: %s", this->oid_.c_str());
    waiting_response_ = false;
    this->publish_state(NAN);
    
    // DŮLEŽITÉ: Vyčistit UDP buffer
    while (udp_.parsePacket() > 0) {
      udp_.flush();
    }
    return;
  }
  
  this->handle_response();
}

void SnmpSensor::send_snmp_get() {
  IPAddress ip;
  if (!ip.fromString(this->host_.c_str())) {
    ESP_LOGE(TAG, "Invalid IP address: %s", this->host_.c_str());
    return;
  }

  // Vytvoření SNMP packetu
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
  
  // OID
  std::string oid_bytes = this->oid_to_bytes(this->oid_);
  packet.push_back(0x06);
  packet.push_back(oid_bytes.length());
  packet.insert(packet.end(), oid_bytes.begin(), oid_bytes.end());
  
  // Value (null)
  packet.insert(packet.end(), {0x05, 0x00});
  
  // Odeslání packetu s lepším error handlingem
  if (this->udp_.beginPacket(this->host_.c_str(), this->port_)) {
    size_t written = this->udp_.write(packet.data(), packet.size());
    if (written == packet.size()) {
      if (this->udp_.endPacket()) {
        this->waiting_response_ = true;
        this->send_time_ = millis();
        ESP_LOGD(TAG, "SNMP query sent to %s", this->host_.c_str());
      } else {
        ESP_LOGE(TAG, "Failed to end packet");
        waiting_response_ = false;
      }
    } else {
      ESP_LOGE(TAG, "Failed to write packet: %d/%d", written, packet.size());
      waiting_response_ = false;
    }
  } else {
    ESP_LOGE(TAG, "Failed to begin packet");
    waiting_response_ = false;
  }
}

void SnmpSensor::handle_response() {
  int packet_size = this->udp_.parsePacket();
  if (packet_size > 0) {
    uint8_t buffer[128]; // Zmenšený buffer
    int len = this->udp_.read(buffer, sizeof(buffer));
    
    if (len > 0) {
      float value = this->parse_snmp_response(buffer, len);
      this->publish_state(value);
      ESP_LOGD(TAG, "SNMP response: %.2f", value);
    }
    this->waiting_response_ = false;
    
    // Vyčistit jakákoliv zbývající data
    while (udp_.parsePacket() > 0) {
      udp_.flush();
    }
  }
}

std::string SnmpSensor::oid_to_bytes(const std::string &oid) {
  if (oid == "1.3.6.1.4.1.318.1.1.1.3.2.1.0") {
    return std::string("\x2b\x06\x01\x04\x01\x4e\x01\x01\x01\x03\x02\x01\x00", 13);
  } else if (oid == "1.3.6.1.4.1.318.1.1.1.2.2.3.0") {
    return std::string("\x2b\x06\x01\x04\x01\x4e\x01\x01\x01\x02\x02\x03\x00", 13);
  } else if (oid == "1.3.6.1.4.1.318.1.1.1.2.2.1.0") {
    return std::string("\x2b\x06\x01\x04\x01\x4e\x01\x01\x01\x02\x02\x01\x00", 13);
  }
  
  // Fallback pro jiné OID
  ESP_LOGW(TAG, "Using fallback OID conversion for: %s", oid.c_str());
  return std::string("\x2b\x06\x01\x04\x01\x4e", 6); // 1.3.6.1.4.1.318
}

float SnmpSensor::parse_snmp_response(const uint8_t *buffer, size_t length) {
  for (size_t i = 0; i < length - 2; i++) {
    if (buffer[i] == 0x02) { // INTEGER
      uint8_t len = buffer[i+1];
      if (len == 0x04 && i + 6 < length) {
        int32_t value = (buffer[i+2] << 24) | (buffer[i+3] << 16) | 
                       (buffer[i+4] << 8) | buffer[i+5];
        return static_cast<float>(value);
      }
    }
  }
  return NAN;
}

}  // namespace snmp
}  // namespace esphome
