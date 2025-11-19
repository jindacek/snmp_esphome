#include "snmp_sensor.h"
#include <lwip/sockets.h>
#include <lwip/netdb.h>
#include <errno.h>

namespace esphome {
namespace snmp {

static const char *const TAG = "snmp";

void SnmpSensor::update() {
  if (!network::is_connected()) {
    ESP_LOGW(TAG, "WiFi not connected, skipping SNMP update");
    this->publish_state(NAN);
    return;
  }
  
  ESP_LOGD(TAG, "Sending SNMP query for OID: %s", this->oid_.c_str());
  
  if (this->send_snmp_query()) {
    ESP_LOGD(TAG, "SNMP query successful");
  } else {
    ESP_LOGE(TAG, "SNMP query failed");
    this->publish_state(NAN);
  }
}

bool SnmpSensor::send_snmp_query() {
  // Resolve hostname to IP address
  struct hostent *server = gethostbyname(this->host_.c_str());
  if (server == NULL) {
    ESP_LOGE(TAG, "Failed to resolve host: %s", this->host_.c_str());
    return false;
  }

  // Create socket
  int sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (sockfd < 0) {
    ESP_LOGE(TAG, "Failed to create socket: %d", errno);
    return false;
  }

  // Set timeout (3 seconds)
  struct timeval timeout;
  timeout.tv_sec = 3;
  timeout.tv_usec = 0;
  setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

  // Setup destination address
  struct sockaddr_in dest_addr;
  memset(&dest_addr, 0, sizeof(dest_addr));
  dest_addr.sin_family = AF_INET;
  dest_addr.sin_port = htons(this->port_);
  memcpy(&dest_addr.sin_addr.s_addr, server->h_addr, server->h_length);

  // Build SNMP packet
  std::vector<uint8_t> packet = this->build_snmp_packet();
  if (packet.empty()) {
    ESP_LOGE(TAG, "Failed to build SNMP packet");
    close(sockfd);
    return false;
  }

  // Send packet
  ssize_t sent_bytes = sendto(sockfd, packet.data(), packet.size(), 0,
                             (struct sockaddr*)&dest_addr, sizeof(dest_addr));
  if (sent_bytes < 0) {
    ESP_LOGE(TAG, "Failed to send SNMP packet: %d", errno);
    close(sockfd);
    return false;
  }

  ESP_LOGD(TAG, "Sent %d bytes to %s", sent_bytes, this->host_.c_str());

  // Receive response
  uint8_t buffer[512];
  struct sockaddr_in src_addr;
  socklen_t src_addr_len = sizeof(src_addr);
  
  ssize_t received_bytes = recvfrom(sockfd, buffer, sizeof(buffer), 0,
                                   (struct sockaddr*)&src_addr, &src_addr_len);
  
  close(sockfd); // Close socket immediately after receiving

  if (received_bytes < 0) {
    if (errno == EAGAIN || errno == EWOULDBLOCK) {
      ESP_LOGW(TAG, "SNMP timeout for %s", this->host_.c_str());
    } else {
      ESP_LOGE(TAG, "Failed to receive SNMP response: %d", errno);
    }
    return false;
  }

  ESP_LOGD(TAG, "Received %d bytes from %s", received_bytes, this->host_.c_str());

  // Parse response
  float value = this->parse_snmp_response(buffer, received_bytes);
  if (!isnan(value)) {
    this->publish_state(value);
    return true;
  } else {
    ESP_LOGE(TAG, "Failed to parse SNMP response");
    return false;
  }
}

std::vector<uint8_t> SnmpSensor::build_snmp_packet() {
  std::vector<uint8_t> packet;
  
  try {
    // SNMP Message Sequence
    packet.insert(packet.end(), {0x30, 0x29}); // SEQUENCE, length 41 bytes
    
    // Version: INTEGER 0 (SNMPv1)
    packet.insert(packet.end(), {0x02, 0x01, 0x00});
    
    // Community: OCTET STRING
    packet.push_back(0x04); // OCTET STRING
    packet.push_back(this->community_.length());
    packet.insert(packet.end(), this->community_.begin(), this->community_.end());
    
    // PDU: Get Request
    packet.insert(packet.end(), {0xA0, 0x1C}); // GetRequest PDU, length 28 bytes
    
    // Request ID
    packet.insert(packet.end(), {0x02, 0x04, 0x00, 0x00, 0x00, 0x01});
    
    // Error status
    packet.insert(packet.end(), {0x02, 0x01, 0x00});
    
    // Error index
    packet.insert(packet.end(), {0x02, 0x01, 0x00});
    
    // Variable bindings
    packet.insert(packet.end(), {0x30, 0x0E}); // SEQUENCE, length 14 bytes
    
    // Single variable binding
    packet.insert(packet.end(), {0x30, 0x0C}); // SEQUENCE, length 12 bytes
    
    // OID
    std::string oid_bytes = this->oid_to_bytes(this->oid_);
    packet.push_back(0x06); // OBJECT IDENTIFIER
    packet.push_back(oid_bytes.length());
    packet.insert(packet.end(), oid_bytes.begin(), oid_bytes.end());
    
    // Value (null)
    packet.insert(packet.end(), {0x05, 0x00}); // NULL
    
  } catch (const std::exception& e) {
    ESP_LOGE(TAG, "Exception building SNMP packet: %s", e.what());
    return std::vector<uint8_t>();
  }
  
  return packet;
}

std::string SnmpSensor::oid_to_bytes(const std::string &oid) {
  // Hardcoded OID conversions for common APC UPS OIDs
  if (oid == "1.3.6.1.4.1.318.1.1.1.3.2.1.0") {
    // Battery Voltage
    return std::string("\x2b\x06\x01\x04\x01\x4e\x01\x01\x01\x03\x02\x01\x00", 13);
  } else if (oid == "1.3.6.1.4.1.318.1.1.1.2.2.3.0") {
    // Runtime
    return std::string("\x2b\x06\x01\x04\x01\x4e\x01\x01\x01\x02\x02\x03\x00", 13);
  } else if (oid == "1.3.6.1.4.1.318.1.1.1.2.2.1.0") {
    // Battery Charge
    return std::string("\x2b\x06\x01\x04\x01\x4e\x01\x01\x01\x02\x02\x01\x00", 13);
  } else if (oid == "1.3.6.1.4.1.318.1.1.1.4.2.3.0") {
    // UPS Load
    return std::string("\x2b\x06\x01\x04\x01\x4e\x01\x01\x01\x04\x02\x03\x00", 13);
  }
  
  // Fallback for unknown OIDs - try basic conversion
  ESP_LOGW(TAG, "Unknown OID, using fallback conversion: %s", oid.c_str());
  std::vector<uint8_t> result;
  
  // Basic OID conversion (this is simplified and may not work for all OIDs)
  std::istringstream iss(oid);
  std::string token;
  std::vector<int> oid_parts;
  
  while (std::getline(iss, token, '.')) {
    if (!token.empty()) {
      oid_parts.push_back(std::stoi(token));
    }
  }
  
  if (oid_parts.size() >= 2) {
    // First two parts are encoded as 40 * X + Y
    result.push_back(40 * oid_parts[0] + oid_parts[1]);
    
    // Remaining parts
    for (size_t i = 2; i < oid_parts.size(); i++) {
      int value = oid_parts[i];
      if (value < 128) {
        result.push_back(value);
      } else {
        // Multi-byte encoding needed (simplified)
        while (value > 0) {
          result.push_back(value & 0x7F);
          value >>= 7;
        }
      }
    }
  }
  
  return std::string(result.begin(), result.end());
}

float SnmpSensor::parse_snmp_response(const uint8_t *buffer, size_t length) {
  if (length < 10) {
    ESP_LOGE(TAG, "SNMP response too short: %d bytes", length);
    return NAN;
  }

  // Simple parser - look for integer values in the response
  for (size_t i = 0; i < length - 2; i++) {
    if (buffer[i] == 0x02) { // INTEGER tag
      uint8_t value_length = buffer[i + 1];
      
      if (value_length == 1 && i + 2 < length) {
        // 1-byte integer
        return static_cast<float>(static_cast<int8_t>(buffer[i + 2]));
      } else if (value_length == 2 && i + 3 < length) {
        // 2-byte integer
        int16_t value = (buffer[i + 2] << 8) | buffer[i + 3];
        return static_cast<float>(value);
      } else if (value_length == 4 && i + 5 < length) {
        // 4-byte integer
        int32_t value = (buffer[i + 2] << 24) | (buffer[i + 3] << 16) | 
                       (buffer[i + 4] << 8) | buffer[i + 5];
        return static_cast<float>(value);
      }
    }
  }
  
  ESP_LOGE(TAG, "No integer value found in SNMP response");
  return NAN;
}

}  // namespace snmp
}  // namespace esphome
