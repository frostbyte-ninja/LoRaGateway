#pragma once

#include <Arduino.h>

#include <stdint.h>

#include <PubSubClient.h>
#include <WiFiClient.h>

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
             uint16_t mqttPort = 1883) noexcept;
  ~MqttClient();

  bool publish(const String& topic, const String& payload, bool retained = true);
  void loop();

private:
  void reconnect();
  void wifiReconnect() const;

  String m_ssid;
  String m_wifiPassword;
  String m_mqttUsername;
  String m_mqttPassword;
  String m_clientId;
  String m_mqttServer;

  WiFiClient m_wifiClient;
  PubSubClient m_pubSubClient;
};
} // namespace mqtt
