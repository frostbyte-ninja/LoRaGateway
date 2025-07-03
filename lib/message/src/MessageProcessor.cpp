#include <message/MessageProcessor.h>

#include <utility>

namespace message {
namespace {
constexpr auto g_mqttSensorTopic{"homeassistant/sensor/"};
constexpr auto g_mqttBinarySensorTopic{"homeassistant/binary_sensor/"};

constexpr auto g_payloadOn{"on"};
constexpr auto g_payloadOff{"off"};

enum class ValueType : uint8_t
{
  Integer,
  Float,
  String,
};

struct DiscoveryInfo
{
  const char* key;
  const char* name;
  const char* uniqueIdSuffix;
  const char* topicPrefix;
  const char* topicSuffix;
  const char* deviceClass;
  const char* unitOfMeasurement;
  const char* icon;
  const char* entityCategory;
  const char* payloadOn;
  const char* payloadOff;
  ValueType valueType;
};

// clang-format off
constexpr DiscoveryInfo g_discoveryInfos[] {
  {"b", "Battery", "_batt", g_mqttSensorTopic, "/batt", "battery", "%", "mdi:battery", "diagnostic", nullptr, nullptr, ValueType::Integer},
  {"r", "RSSI", "_rssi", g_mqttSensorTopic, "/rssi", "signal_strength", "dBm", "mdi:signal", "diagnostic", nullptr, nullptr, ValueType::Integer},
  {"rw", "Text", "_row", g_mqttSensorTopic, "/row", nullptr, nullptr, "mdi:text", nullptr, nullptr, nullptr, ValueType::String},
  {"s", "State", "_state", g_mqttSensorTopic, "/state", nullptr, nullptr, "mdi:list-status", nullptr, nullptr, nullptr, ValueType::String},
  {"v", "Volt", "_volt", g_mqttSensorTopic, "/volt", "voltage", "V", "mdi:flash-triangle", nullptr, nullptr, nullptr, ValueType::Float},
  {"pw", "Current", "_pw", g_mqttSensorTopic, "/current", "current", "mA", "mdi:current-dc", nullptr, nullptr, nullptr, ValueType::Float},
  {"l", "Lux", "_lx", g_mqttSensorTopic, "/lx", "illuminance", "lx", "mdi:brightness-1", nullptr, nullptr, nullptr, ValueType::Integer},
  {"w", "Weight", "_w", g_mqttSensorTopic, "/weight", "weight", "g", "mdi:weight", nullptr, nullptr, nullptr, ValueType::Float},
  {"t", "Temperature", "_tmp", g_mqttSensorTopic, "/tmp", "temperature", "°C", "mdi:thermometer", nullptr, nullptr, nullptr, ValueType::Float},
  {"t2", "Temperature2", "_tmp2", g_mqttSensorTopic, "/tmp2", "temperature", "°C", "mdi:thermometer", nullptr, nullptr, nullptr, ValueType::Float},
  {"hu", "Humidity", "_hu", g_mqttSensorTopic, "/humidity", "humidity", "%", "mdi:water-percent", nullptr, nullptr, nullptr, ValueType::Float},
  {"mo", "Moisture", "_mo", g_mqttSensorTopic, "/moisture", "moisture", "%", "mdi:water-percent", nullptr, nullptr, nullptr, ValueType::Float},
  {"bt", "Button", "_bt", g_mqttBinarySensorTopic, "/button", "none", nullptr, "mdi:button", nullptr, g_payloadOn, g_payloadOff, ValueType::String},
  {"atm", "Pressure", "_atm", g_mqttSensorTopic, "/pressure", "atmospheric_pressure", "kPa", "mdi:button", nullptr, nullptr, nullptr, ValueType::Float},
  {"cd", "Carbon Dioxide", "_cd", g_mqttSensorTopic, "/co2", "carbon_dioxide", "ppm", "mdi:molecule-co2", nullptr, nullptr, nullptr, ValueType::Integer},
  {"m", "Motion", "_m", g_mqttBinarySensorTopic, "/motion", "motion", nullptr, "mdi:motion", nullptr, g_payloadOn, g_payloadOff, ValueType::String},
  {"dr", "Door", "_door", g_mqttBinarySensorTopic, "/door", "door", nullptr, "mdi:door", nullptr, g_payloadOn, g_payloadOff, ValueType::String},
  {"wd", "Window", "_window", g_mqttBinarySensorTopic, "/window", "window", nullptr, "mdi:window-closed", nullptr, g_payloadOn, g_payloadOff, ValueType::String},
  {"vb", "Vibration", "_vibration", g_mqttBinarySensorTopic, "/vibration", "vibration", nullptr, "mdi:vibrate", nullptr, g_payloadOn, g_payloadOff, ValueType::String},
};
// clang-format on

void
fillJsonDoc(JsonDocument& json, const DiscoveryInfo* info, const String& nodeId)
{
  json["name"] = info->name;
  json["unique_id"] = nodeId + info->uniqueIdSuffix;
  json["state_topic"] = String{info->topicPrefix} + nodeId + info->topicSuffix;
  if (info->deviceClass != nullptr) {
    json["device_class"] = info->deviceClass;
  }
  if (info->unitOfMeasurement != nullptr) {
    json["unit_of_meas"] = info->unitOfMeasurement;
  }
  if (info->icon != nullptr) {
    json["icon"] = info->icon;
  }
  if (info->entityCategory != nullptr) {
    json["ent_cat"] = info->entityCategory;
  }
  if (info->payloadOn != nullptr) {
    json["payload_on"] = info->payloadOn;
  }
  if (info->payloadOff != nullptr) {
    json["payload_off"] = info->payloadOff;
  }

  const auto device{json["device"].to<JsonObject>()};
  const auto identifiers{device["ids"].to<JsonArray>()};
  // ReSharper disable once CppExpressionWithoutSideEffects
  identifiers.add(nodeId);
  device["name"] = nodeId;
  device["mdl"] = nodeId;
  device["mf"] = "PricelessToolkit";
}

const DiscoveryInfo*
getDiscoveryInfo(const String& key)
{
  for (const auto& info : g_discoveryInfos) {
    if (key == info.key) {
      return &info;
    }
  }
  return nullptr;
}

std::optional<String>
convertToString(const JsonDocument& doc, const String& key, const ValueType valueType)
{
  switch (valueType) {
    case ValueType::Integer:
      if (doc[key].is<int>()) {
        return String{doc[key].as<int>()};
      }
      break;
    case ValueType::Float:
      if (doc[key].is<float>()) {
        return String{doc[key].as<float>(), 2};
      }
      break;
    case ValueType::String:
      if (doc[key].is<const char*>() || doc[key].is<String>()) {
        return doc[key].as<String>();
      }
      break;
  }
  return std::nullopt;
}
} // namespace

MessageProcessor::MessageProcessor(PublishCallback publish, RssiCallback rssi, String gatewayId) noexcept
  : m_publish{std::move(publish)}
  , m_rssi{std::move(rssi)}
  , m_gatewayId{std::move(gatewayId)}
{
}

void
MessageProcessor::processMessage(const String& message) const
{
  if (message.isEmpty()) {
    return;
  }
  Serial.print(F("Received message: "));
  Serial.println(message);

  JsonDocument doc;
  if (const auto error{deserializeJson(doc, message)}) {
    Serial.print(F("Failed to deserialize JSON: "));
    Serial.println(error.f_str());
    return;
  }

  if (not doc["k"].is<String>()) {
    Serial.println(F("Gateway key not found"));
    return;
  }

  if (doc["k"].as<String>() != m_gatewayId) {
    return;
  }

  if (not doc["id"].is<String>()) {
    Serial.println(F("No or invalid node ID"));
    return;
  }

  doc["r"] = m_rssi();

  publishUpdate(doc);
}

void
MessageProcessor::publishDiscoveryMessage(const String& key, const String& nodeId) const
{
  const DiscoveryInfo* info{getDiscoveryInfo(key)};
  if (info == nullptr) {
    Serial.print(F("Unknown key: "));
    Serial.println(key);
    return;
  }

  JsonDocument json;
  fillJsonDoc(json, info, nodeId);

  String payload;
  serializeJson(json, payload);
  const String topic{String{info->topicPrefix} + nodeId + info->topicSuffix + "/config"};
  if (not m_publish(topic.c_str(), payload.c_str(), true)) {
    Serial.println(F("publish failed"));
  }
}

void
MessageProcessor::publishUpdate(const JsonDocument& doc) const
{
  for (const auto& info : g_discoveryInfos) {
    const String key{info.key};
    const auto payload{convertToString(doc, key, info.valueType)};
    if (not payload.has_value()) {
      continue;
    }

    const auto identifier{doc["id"].as<String>()};
    publishDiscoveryMessage(key, identifier);

    const String topic{String{info.topicPrefix} + identifier + info.topicSuffix};
    if (not m_publish(topic.c_str(), payload->c_str(), true)) {
      Serial.println(F("publish failed"));
    }
  }
}
} // namespace message
