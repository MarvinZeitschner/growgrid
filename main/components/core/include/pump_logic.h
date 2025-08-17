#pragma once
#include <stdbool.h>

#define PUMP_LOGIC_MOISTURE_THRESHOLD_PERCENT 30

/**
 * @brief Determines if the pump should be activated based on soil moisture.
 *
 * @param moisture_percent The current soil moisture in percent.
 * @return true if the pump should be started, false otherwise.
 */
bool pump_logic_should_start(int moisture_percent);
