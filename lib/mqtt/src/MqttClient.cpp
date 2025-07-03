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
                       const uint16_t mqttPort) noexcept
  : m_ssid{std::move(ssid)}
  , m_wifiPassword{std::move(wifiPassword)}
  , m_mqttUsername{std::move(mqttUsername)}
  , m_mqttPassword{std::move(mqttPassword)}
  , m_clientId{std::move(clientId)}
  , m_mqttServer{std::move(mqttServer)}
  , m_pubSubClient{m_wifiClient}
{
  m_pubSubClient.setBufferSize(1024);
  m_pubSubClient.setServer(m_mqttServer.c_str(), mqttPort);
}

MqttClient::~MqttClient() = default;

void
MqttClient::loop()
{
  reconnect();
  m_pubSubClient.loop();
}

bool
MqttClient::publish(const String& topic, const String& payload, const bool retained)
{
  reconnect();
  return m_pubSubClient.publish(topic.c_str(), payload.c_str(), retained);
}

void
MqttClient::reconnect()
{
  while (not m_pubSubClient.connected()) {
    wifiReconnect();

    if (m_pubSubClient.connect(m_clientId.c_str(), m_mqttUsername.c_str(), m_mqttPassword.c_str())) {
      Serial.println("MQTT connected");
    } else {
      Serial.print("MQTT failed, state: ");
      Serial.println(m_pubSubClient.state());
      delay(1000);
    }
  }
}

void
MqttClient::wifiReconnect() const
{
  if (WiFi.status() == WL_CONNECTED) { // NOLINT (*-static-accessed-through-instance)
    return;
  }

  WiFi.begin(m_ssid, m_wifiPassword);
  while (WiFi.status() != WL_CONNECTED) { // NOLINT (*-static-accessed-through-instance)
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}
} // namespace mqtt
