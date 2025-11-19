#include "snmp_sensor.h"
#include <lwip/sockets.h>
#include <lwip/netdb.h>

namespace esphome {
namespace snmp {

static const char *const TAG = "snmp";

void SnmpSensor::setup() {
  // Inicializace až v update() když je WiFi připojeno
}

void SnmpSensor::update() {
  if (!network::is_connected()) {
    ESP_LOGW(TAG, "WiFi not connected");
    return;
  }
  
  this->send_snmp_get();
}

void SnmpSensor::send_snmp_get() {
  struct sockaddr_in dest_addr;
  dest_addr.sin_addr.s_addr = inet_addr(this->host_.c_str());
  dest_addr.sin_family = AF_INET;
  dest_addr.sin_port = htons(this->port_);
  
  int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
  if (sock < 0) {
    ESP_LOGE(TAG, "Unable to create socket");
    return;
  }
  
  // Timeout pro socket
  struct timeval timeout;
  timeout.tv_sec = 3;
  timeout.tv_usec = 0;
  setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
  
  // Vytvoření SNMP packetu (stejné jako předtím)
  std::vector<uint8_t> packet;
  // ... [stejný SNMP packet code] ...
  
  // Odeslání
  int err = sendto(sock, packet.data(), packet.size(), 0, 
                   (struct sockaddr *)&dest_addr, sizeof(dest_addr));
  if (err < 0) {
    ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
    close(sock);
    return;
  }
  
  // Příjem odpovědi
  uint8_t rx_buffer[128];
  int len = recv(sock, rx_buffer, sizeof(rx_buffer) - 1, 0);
  
  if (len > 0) {
    float value = this->parse_snmp_response(rx_buffer, len);
    this->publish_state(value);
  } else {
    ESP_LOGW(TAG, "SNMP timeout");
    this->publish_state(NAN);
  }
  
  close(sock);
}
