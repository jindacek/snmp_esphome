#include "SNMPManager.h"

SNMPManager::SNMPManager() {}

bool SNMPManager::begin() {
  return udp_.begin(161);  // port SNMP client
}

bool SNMPManager::get(const char* host, const char* community, const char* oid, long* value) {
  IPAddress ip;
  if (!ip.fromString(host)) return false;

  SNMPPacket packet;
  packet.setVersion(0);  // SNMPv1
  packet.setCommunity(community);
  packet.addGetRequest(oid);

  udp_.beginPacket(ip, 161);
  udp_.write(packet.data(), packet.length());
  udp_.endPacket();

  delay(50);

  int len = udp_.parsePacket();
  if (len <= 0) return false;

  uint8_t buffer[1024];
  udp_.read(buffer, len);

  SNMPVariable var;
  if (!SNMPPacket::parseResponse(buffer, len, &var)) return false;

  *value = var.asInteger();
  return true;
}
