#pragma once

/**
 * @brief Initializes SNTP and waits for time synchronization.
 * 
 * This function is blocking and will not return until the time is set.
 */
void platform_sntp_init(void);
