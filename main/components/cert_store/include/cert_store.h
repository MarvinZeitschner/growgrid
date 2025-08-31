#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

extern const uint8_t ca_cert_pem_start[] asm("_binary_ca_crt_start");
extern const uint8_t client_cert_pem_start[] asm("_binary_esp32_crt_start");
extern const uint8_t client_key_pem_start[] asm("_binary_esp32_key_start");

#ifdef __cplusplus
}
#endif
