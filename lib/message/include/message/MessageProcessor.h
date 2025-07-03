#pragma once

#include <Arduino.h>

#include <functional>

#include <ArduinoJson.h>

namespace message {
class MessageProcessor
{
public:
  using PublishCallback = std::function<bool(const String& topic, const String& payload, bool retained)>;
  using RssiCallback = std::function<int()>;

  MessageProcessor(PublishCallback publish, RssiCallback rssi, String gatewayId) noexcept;

  void processMessage(const String& message) const;

private:
  PublishCallback m_publish;
  RssiCallback m_rssi;
  String m_gatewayId;

  void publishDiscoveryMessage(const String& key, const String& nodeId) const;
  void publishUpdate(const JsonDocument& doc) const;
};
} // namespace message
