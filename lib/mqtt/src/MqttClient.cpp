#include <mqtt/MqttClient.h>

#include <utility>

#include <WiFi.h>

namespace mqtt {
MqttClient::MqttClient(String ssid,
                       String wifiPassword,
                       String mqttUsername,
                       String mqttPassword,
                       String clientId,
                       String mqttServer,
                       const std::uint16_t mqttPort) noexcept
  : m_ssid{std::move(ssid)}
  , m_wifiPassword{std::move(wifiPassword)}
  , m_mqttUsername{std::move(mqttUsername)}
  , m_mqttPassword{std::move(mqttPassword)}
  , m_clientId{std::move(clientId)}
  , m_mqttServer{std::move(mqttServer)}
{
  WiFi.setSleep(false);

  m_mqttClient.setCredentials(m_mqttUsername.c_str(), m_mqttPassword.c_str());
  m_mqttClient.setClientId(m_clientId.c_str());
  m_mqttClient.setServer(m_mqttServer.c_str());
  m_mqttClient.getMqttConfig()->broker.address.port = mqttPort;

  m_mqttClient.setAutoReconnect(true);
  m_mqttClient.onConnect([](bool) {
    Serial.println(F("MQTT connected"));
  });
  m_mqttClient.onDisconnect([](bool) {
    Serial.println(F("MQTT disconnected"));
  });
}

bool
MqttClient::publish(const String& topic, const String& payload, const bool retain)
{
  return m_mqttClient.publish(topic.c_str(), 2, retain, payload.c_str(), static_cast<int>(payload.length())) != -1;
}

void
MqttClient::connect()
{
  Serial.println(F("WiFi connecting..."));
  WiFi.begin(m_ssid, m_wifiPassword);
  while (not WiFi.isConnected()) {
    delay(500);
  }
  Serial.println(F("WiFi connected"));
  Serial.print(F("IP address: "));
  Serial.println(WiFi.localIP());

  Serial.println(F("MQTT connecting..."));
  m_mqttClient.connect();
  while (not m_mqttClient.connected()) {
    delay(500);
  }
}
} // namespace mqtt
