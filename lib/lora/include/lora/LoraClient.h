#pragma once

#include <Arduino.h>

#include <optional>

#include <RadioLib.h>
#include <aes/Aes.hpp>

namespace lora {
class LoraClient
{
public:
  using PacketReceivedAction = void (*)();

  explicit LoraClient(const aes::Aes::Array& key) noexcept;

  bool begin();
  std::optional<String> receiveMessage();
  void setPacketReceivedAction(PacketReceivedAction callback);
  int getRssi();
  int32_t randomInt();

private:
  aes::Aes m_aes;
  Module m_module;
  SX1262 m_lora;
};
} // namespace lora
