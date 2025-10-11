#pragma once

#include <chrono>

namespace watchdog {

class Watchdog
{
public:
  explicit Watchdog(std::chrono::seconds timeout);
  void start();
  void stop();
  void reset();

private:
  std::chrono::seconds m_timeout;
  bool m_started;
};

} // namespace watchdog
