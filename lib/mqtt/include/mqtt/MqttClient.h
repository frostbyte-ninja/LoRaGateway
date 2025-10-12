#pragma once

#include <atomic>
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
  void init();
  void attemptWifiReconnect();

  String m_ssid;
  String m_wifiPassword;
  String m_mqttUsername;
  String m_mqttPassword;
  String m_clientId;
  String m_mqttServer;
  std::uint16_t m_mqttPort;
  std::atomic<std::uint32_t> m_nextReconnectMs;
  bool m_initialized;

  PsychicMqttClient m_mqttClient;
};
} // namespace mqtt
