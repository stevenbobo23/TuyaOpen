/**
 * @file drv_encoder.h
 * @brief Rotary encoder driver interface for Tuya IoT devices.
 *
 * This file provides the interface for rotary encoder functionality in Tuya IoT
 * devices. It supports quadrature encoders with button functionality, enabling
 * accurate position tracking and user input detection. The driver provides
 * thread-safe operations for reading encoder angle values and button states.
 *
 * Key features:
 * - Quadrature encoder angle tracking with interrupt-driven detection
 * - Built-in button support for encoder switches
 * - Thread-safe operations using mutex protection
 * - GPIO-based implementation with configurable pins
 * - Semaphore-based event handling for responsive input processing
 *
 * The driver abstracts the low-level GPIO operations and interrupt handling,
 * providing a simple interface for applications to read encoder position
 * and button states.
 *
 * @copyright Copyright (c) 2021-2025 Tuya Inc. All Rights Reserved.
 *
 */

#ifndef __DRV_ENCODER_H__
#define __DRV_ENCODER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "tuya_cloud_types.h"

#include "tkl_gpio.h"
#include "tal_api.h"
#include "tal_log.h"
#include "tkl_output.h"
/**
 * @brief Get the angle value of the encoder.
 *
 * This function locks the mutex before getting the angle value to ensure thread safety.
 * After reading the angle value, it unlocks the mutex. The returned value is the current
 * angle of the encoder.
 *
 * @return The current angle value of the encoder, in degrees.
 */
int32_t encoder_get_angle(void);

/**
 * @brief Get the angle value of the encoder.
 *
 * This function locks the mutex before getting the angle value to ensure thread safety.
 * After reading the angle value, it unlocks the mutex. The returned value is the current
 * angle of the encoder.
 *
 * @return The current angle value of the encoder, in degrees.
 */
int32_t encoder_get_angle(void);

/**
 * @brief Check if the encoder button is pressed.
 *
 * This function reads the voltage level of the encoder input pin. If a low voltage level
 * is still detected after a short delay, it returns 1 to indicate that the button is pressed;
 * otherwise, it returns 0 to indicate that it is not pressed.
 *
 * @return uint8_t Returns 1 if the button is pressed, returns 0 if not pressed.
 */
uint8_t encoder_get_pressed(void);

/**
 * @brief Initialize the encoder module.
 *
 * This function is responsible for creating semaphores and mutexes, initializing GPIO input pins,
 * and setting up the encoder input interrupt configuration. If the waiting thread has not been
 * created yet, it will start the thread to handle semaphore waiting.
 *
 * @return None
 */
void tkl_encoder_init(void);

#ifdef __cplusplus
}
#endif
#endif /* __DRV_ENCODER_H__ */