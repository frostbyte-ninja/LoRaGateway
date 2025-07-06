#include <lora/LoraClient.h>

#include <algorithm>
#include <cctype>
#include <limits>
#include <vector>

#include <SPI.h>

namespace lora {
namespace {
constexpr uint8_t g_spiSckPin{5U};
constexpr uint8_t g_spiMisoPin{3U};
constexpr uint8_t g_spiMosiPin{6U};

constexpr uint8_t g_radioNssPin{7U};
constexpr uint8_t g_radioDio1Pin{33U};
constexpr uint8_t g_radioResetPin{8U};
constexpr uint8_t g_radioBusyPin{34U};

constexpr float g_loraFrequency{868.0F};
constexpr float g_bandwidth{125.0F};
constexpr uint8_t g_spreadingFactor{8U};
constexpr uint8_t g_codingRate{5U};
constexpr uint8_t g_syncWord{RADIOLIB_SX126X_SYNC_WORD_PRIVATE};
constexpr uint8_t g_power{20U};
constexpr uint16_t g_preambleLength{6U};
constexpr float g_txcoVoltage{1.6F};

std::optional<String>
toString(const char* const array, const std::size_t length)
{
  const bool isPrintable{std::all_of(array, array + length, [](const char character) {
    return std::isprint(character);
  })};

  return isPrintable ? std::make_optional(String{array, length}) : std::nullopt;
}

} // namespace

LoraClient::LoraClient(const crypto::Aes::Array& key) noexcept
  : m_cipher{key}
  , m_module{g_radioNssPin, g_radioDio1Pin, g_radioResetPin, g_radioBusyPin, SPI}
  , m_lora{&m_module}
{
}

bool
LoraClient::begin()
{
  SPI.begin(g_spiSckPin, g_spiMisoPin, g_spiMosiPin);
  if (const auto state{m_lora.begin(g_loraFrequency,
                                    g_bandwidth,
                                    g_spreadingFactor,
                                    g_codingRate,
                                    g_syncWord,
                                    g_power,
                                    g_preambleLength,
                                    g_txcoVoltage)};
      state != RADIOLIB_ERR_NONE) {
    Serial.print(F("LoRa begin failed, code: "));
    Serial.println(state);
    return false;
  }

  if (const auto state{m_lora.setCRC(true)}; state != RADIOLIB_ERR_NONE) {
    Serial.print(F("Failed to set CRC, code: "));
    Serial.println(state);
    return false;
  }

  if (const auto state{m_lora.invertIQ(false)}; state != RADIOLIB_ERR_NONE) {
    Serial.print(F("Failed to set invert IQ, code: "));
    Serial.println(state);
    return false;
  }

  if (const auto state{m_lora.explicitHeader()}; state != RADIOLIB_ERR_NONE) {
    Serial.print(F("Failed to set explicit header, code: "));
    Serial.println(state);
    return false;
  }

  return true;
}

bool
LoraClient::startReceive()
{
  if (const auto state{m_lora.startReceive()}; state != RADIOLIB_ERR_NONE) {
    Serial.print(F("Failed to start receive, code: "));
    Serial.println(state);
    return false;
  }

  return true;
}

std::optional<String>
LoraClient::receiveMessage()
{
  const auto packetLength{m_lora.getPacketLength()};
  if (packetLength == 0) {
    return std::nullopt;
  }

  std::vector<byte> receiveBuffer(packetLength);
  if (const auto state{m_lora.readData(receiveBuffer.data(), receiveBuffer.size())}; state != RADIOLIB_ERR_NONE) {
    Serial.print(F("Failed to read data, code: "));
    Serial.println(state);
    return std::nullopt;
  }

  std::vector<byte> decryptedMessageBuffer(receiveBuffer.size());
  const auto decryptedSize{m_cipher.decrypt(receiveBuffer.data(), receiveBuffer.size(), decryptedMessageBuffer.data())};

  if (decryptedSize == 0 or decryptedSize > packetLength) {
    Serial.println(F("Failed to decrypt data"));
    return std::nullopt;
  }

  const auto string{toString(reinterpret_cast<const char*>(decryptedMessageBuffer.data()), decryptedSize)};

  if (not string) {
    Serial.println(F("Failed to decrypt data"));
  }

  return string;
}

void
LoraClient::setPacketReceivedAction(const PacketReceivedAction callback)
{
  m_lora.setPacketReceivedAction(callback);
}

int
LoraClient::getRssi()
{
  return static_cast<int>(m_lora.getRSSI());
}

int32_t
LoraClient::randomInt()
{
  return m_lora.random(std::numeric_limits<int32_t>::max());
}
} // namespace lora
