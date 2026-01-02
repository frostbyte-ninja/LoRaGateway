#include <mqtt/MqttClient.h>

#include <chrono>
#include <utility>

#include <WiFi.h>

namespace mqtt {

using namespace std::chrono_literals;

namespace {
constexpr std::chrono::milliseconds g_reconnectBackoff{15s};

const char*
disconnectReasonFor(const std::uint8_t reasonCode)
{
  return WiFi.disconnectReasonName(static_cast<wifi_err_reason_t>(reasonCode));
}
} // namespace

MqttClient::MqttClient(String ssid,
                       String wifiPassword,
                       String mqttUsername,
                       String mqttPassword,
                       String clientId,
                       String mqttServer,
                       const std::uint16_t mqttPort,
                       std::optional<TlsConfig> tlsConfig) noexcept
  : m_ssid{std::move(ssid)}
  , m_wifiPassword{std::move(wifiPassword)}
  , m_mqttUsername{std::move(mqttUsername)}
  , m_mqttPassword{std::move(mqttPassword)}
  , m_clientId{std::move(clientId)}
  , m_mqttServer{std::move(mqttServer)}
  , m_mqttPort{mqttPort}
  , m_tlsConfig{std::move(tlsConfig)}
  , m_nextReconnectMs{0U}
  , m_initialized{false}
{
  // WiFi stuff must not be done here
}

bool
MqttClient::publish(const String& topic, const String& payload, const bool retain)
{
  return m_mqttClient.publish(topic.c_str(), 2, retain, payload.c_str(), static_cast<int>(payload.length())) != -1;
}

void
MqttClient::connect()
{
  init();

  Serial.println(F("WiFi connecting..."));
  WiFi.begin(m_ssid, m_wifiPassword);
  while (not WiFi.isConnected()) {
    delay(500);
  }

  Serial.println(F("MQTT connecting..."));
  m_mqttClient.connect();
  while (not m_mqttClient.connected()) {
    delay(500);
  }
}

void
MqttClient::init()
{
  if (m_initialized) {
    return;
  }

  WiFi.persistent(false);
  WiFi.setAutoReconnect(false); // builtin autoreconnect does not reconnect in some cases
  WiFi.setSleep(false);
  WiFi.onEvent([this](const WiFiEvent_t event, const WiFiEventInfo_t& info) {
    switch (event) {
      case ARDUINO_EVENT_WIFI_STA_CONNECTED: {
        Serial.println(F("WiFi connected"));
      } break;
      case ARDUINO_EVENT_WIFI_STA_GOT_IP: {
        Serial.print(F("IP address: "));
        Serial.println(WiFi.localIP());
      } break;
      case ARDUINO_EVENT_WIFI_STA_DISCONNECTED: {
        Serial.printf(F("WiFi disconnected (Reason: %s)\n"), disconnectReasonFor(info.wifi_sta_disconnected.reason));
        attemptWifiReconnect();
      } break;
      default:;
    }
  });

  m_mqttClient.setCredentials(m_mqttUsername.c_str(), m_mqttPassword.c_str());
  m_mqttClient.setClientId(m_clientId.c_str());
  m_mqttClient.setServer(m_mqttServer.c_str());
  m_mqttClient.getMqttConfig()->broker.address.port = m_mqttPort;

  if (m_tlsConfig) {
    m_mqttClient.setCACert(m_tlsConfig->caCert().c_str());
    if (m_tlsConfig->clientCert() and m_tlsConfig->clientKey()) {
      m_mqttClient.setClientCertificate(m_tlsConfig->clientCert()->c_str(), m_tlsConfig->clientKey()->c_str());
    }
  }

  m_mqttClient.setAutoReconnect(true);
  m_mqttClient.onConnect([](bool) {
    Serial.println(F("MQTT connected"));
  });
  m_mqttClient.onDisconnect([this](bool) {
    // this callback is called cyclically until connected again
    Serial.println(F("MQTT disconnected"));
    if (not WiFi.isConnected()) {
      attemptWifiReconnect();
    }
  });

  m_initialized = true;
}

void
MqttClient::attemptWifiReconnect()
{
  const auto now = millis();

  while (true) {
    auto scheduled = m_nextReconnectMs.load(std::memory_order_relaxed);

    if (static_cast<std::int32_t>(now - scheduled) < 0) {
      return;
    }

    if (const auto newScheduled = now + g_reconnectBackoff.count();
        m_nextReconnectMs.compare_exchange_weak(scheduled, newScheduled, std::memory_order_relaxed)) {
      Serial.println("Attempting reconnect...");
      if (not WiFi.reconnect()) {
        Serial.println("Reconnect failed");
      }
      return;
    }
  }
}
} // namespace mqtt
