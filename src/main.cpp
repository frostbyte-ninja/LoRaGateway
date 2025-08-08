#include <Arduino.h>

#include <lora/LoraClient.h>
#include <message/MessageProcessor.h>
#include <mqtt/MqttClient.h>

namespace {
constexpr auto g_wifiSsid{"xxxxx"};
constexpr auto g_wifiPassword{"xxxxx"};
constexpr auto g_mqttUsername{"xxxxx"};
constexpr auto g_mqttPassword{"xxxxx"};
constexpr auto g_gatewayId{"xy"};
constexpr auto g_mqttServer{"xxxxx"};
constexpr uint16_t g_mqttPort{1883};

constexpr aes::Aes::Array
  g_aesKey{0xC5, 0xBD, 0x18, 0x6E, 0x98, 0xBE, 0x79, 0xF3, 0xFA, 0x98, 0xE3, 0x30, 0xF7, 0x1E, 0x4E, 0x93};

// NOLINTBEGIN(*-avoid-non-const-global-variables,*-err58-cpp)

mqtt::MqttClient
  g_mqttClient{g_wifiSsid, g_wifiPassword, g_mqttUsername, g_mqttPassword, g_gatewayId, g_mqttServer, g_mqttPort};
lora::LoraClient g_loraClient{g_aesKey};
message::MessageProcessor g_jsonProcessor{[](const String& topic, const String& payload, const bool retained) {
                                            return g_mqttClient.publish(topic, payload, retained);
                                          },
                                          [] {
                                            return g_loraClient.getRssi();
                                          },
                                          g_gatewayId};
volatile bool g_messageReceived{false};

// NOLINTEND(*-avoid-non-const-global-variables,*-err58-cpp)

ICACHE_RAM_ATTR
void
messageReceived()
{
  g_messageReceived = true;
}

void
initLoRa()
{
  g_loraClient.setPacketReceivedAction(messageReceived);

  if (not g_loraClient.begin()) {
    // ReSharper disable once CppDFAEndlessLoop
    while (true) {
      yield();
    }
  }
}
} // namespace

void
setup()
{
  Serial.begin(115200);
  delay(500);

  initLoRa();

  g_loraClient.startReceive();
}

void
loop()
{
  g_mqttClient.loop();

  if (g_messageReceived) {
    g_messageReceived = false;

    if (const auto message{g_loraClient.receiveMessage()}) {
      g_jsonProcessor.processMessage(message.value());
    }
  }
}
