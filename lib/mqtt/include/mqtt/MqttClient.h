#pragma once

#include <cstdint>

#include <Arduino.h>

#include <PsychicMqttClient.h>

namespace mqtt {
class MqttClient
{
public:
  MqttClient(String ssid,
             String wifiPassword,
             String mqttUsername,
             String mqttPassword,
             String clientId,
             String mqttServer,
             std::uint16_t mqttPort = 1883) noexcept;
  bool publish(const String& topic, const String& payload, bool retain = true);
  void connect();

private:
  String m_ssid;
  String m_wifiPassword;
  String m_mqttUsername;
  String m_mqttPassword;
  String m_clientId;
  String m_mqttServer;

  PsychicMqttClient m_mqttClient;
};
} // namespace mqtt
