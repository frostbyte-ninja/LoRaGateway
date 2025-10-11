#include <watchdog/Watchdog.h>

#include <esp_task_wdt.h>

namespace watchdog {

Watchdog::Watchdog(const std::chrono::seconds timeout)
  : m_timeout{timeout}
  , m_started{false}
{
}

void
Watchdog::start()
{
  if (m_started) {
    return;
  }

  const esp_task_wdt_config_t config{
    .timeout_ms = static_cast<uint32_t>(std::chrono::duration_cast<std::chrono::milliseconds>(m_timeout).count()),
    .idle_core_mask = (1U << CONFIG_FREERTOS_NUMBER_OF_CORES) - 1U,
    .trigger_panic = true,
  };
  esp_task_wdt_init(&config);
  esp_task_wdt_add(nullptr);
  m_started = true;
}

void
Watchdog::stop()
{
  if (not m_started) {
    return;
  }

  esp_task_wdt_delete(nullptr);
  esp_task_wdt_deinit();
  m_started = false;
}

void
Watchdog::reset()
{
  if (not m_started) {
    return;
  }

  esp_task_wdt_reset();
}

} // namespace watchdog
