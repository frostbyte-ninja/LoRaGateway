#pragma once

#include <atomic>
#include <cstdint>
#include <optional>

#include <Arduino.h>

#include <PsychicMqttClient.h>

namespace mqtt {

class TlsConfig
{
public:
  explicit TlsConfig(String caCert)
    : m_caCert(std::move(caCert))
  {
  }

  TlsConfig(String caCert, String clientCert, String clientKey)
    : m_caCert(std::move(caCert))
    , m_clientCert{std::move(clientCert)}
    , m_clientKey{std::move(clientKey)}
  {
  }

  [[nodiscard]] const String& caCert() const
  {
    return m_caCert;
  }

  [[nodiscard]] const std::optional<String>& clientCert() const
  {
    return m_clientCert;
  }

  [[nodiscard]] const std::optional<String>& clientKey() const
  {
    return m_clientKey;
  }

private:
  String m_caCert;
  std::optional<String> m_clientCert;
  std::optional<String> m_clientKey;
};

class MqttClient
{
public:
  MqttClient(String ssid,
             String wifiPassword,
             String mqttUsername,
             String mqttPassword,
             String clientId,
             String mqttServer,
             std::uint16_t mqttPort = 1883,
             std::optional<TlsConfig> tlsConfig = std::nullopt) noexcept;
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
  std::optional<TlsConfig> m_tlsConfig;
  std::atomic<std::uint32_t> m_nextReconnectMs;
  bool m_initialized;

  PsychicMqttClient m_mqttClient;
};
} // namespace mqtt
