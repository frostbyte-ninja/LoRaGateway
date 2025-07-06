#pragma once

#include <stddef.h>
#include <stdint.h>

#include <array>

#include <Arduino.h>

#include <AESLib.h>

namespace crypto {
class Aes
{
public:
  using Array = std::array<byte, N_BLOCK>;

  explicit Aes(const Array& key, paddingMode paddingMode = paddingMode::CMS) noexcept;
  uint16_t encrypt(const byte* input, uint16_t length, byte* output);
  uint16_t encrypt(const char* input, uint16_t length, char* output);
  uint16_t decrypt(byte* input, uint16_t length, byte* output);
  uint16_t decrypt(char* input, uint16_t length, char* output);
  size_t calculateEncryptedLength(int16_t length);

private:
  AESLib m_aesLib;
  Array m_key;
};
} // namespace crypto
