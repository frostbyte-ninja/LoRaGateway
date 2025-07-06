#pragma once

#include <Arduino.h>

#include <optional>

#include <RadioLib.h>

#include <crypto/Aes.hpp>

namespace lora {
class LoraClient
{
public:
  using PacketReceivedAction = void (*)();

  explicit LoraClient(const crypto::Aes::Array& key) noexcept;

  bool begin();
  bool startReceive();
  std::optional<String> receiveMessage();
  void setPacketReceivedAction(PacketReceivedAction callback);
  int getRssi();
  int32_t randomInt();

private:
  crypto::Aes m_cipher;
  Module m_module;
  SX1262 m_lora;
};
} // namespace lora
