#include "pump_logic.h"

bool pump_logic_should_start(int moisture_percent) {
  return moisture_percent < PUMP_LOGIC_MOISTURE_THRESHOLD_PERCENT;
}
