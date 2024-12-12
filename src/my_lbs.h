/*
 * Copyright (c) 2018 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#ifndef BT_LBS_H_
#define BT_LBS_H_

/**@file
 * @defgroup bt_lbs LED Button Service API
 * @{
 * @brief API for the LED Button Service (LBS).
 */

#ifdef __cplusplus
extern "C" {
#endif

#include <zephyr/types.h>
#define BT_UUID_LBS_VAL BT_UUID_128_ENCODE(0x00001523, 0x1212, 0xefde, 0x1523, 0x785feabcd123)
#define BT_UUID_LBS_BUTTON_VAL BT_UUID_128_ENCODE(0x00001524, 0x1212, 0xefde, 0x1523, 0x785feabcd123)
#define BT_UUID_LBS_LED_VAL BT_UUID_128_ENCODE(0x00001525, 0x1212, 0xefde, 0x1523, 0x785feabcd123)

#define BT_UUID_LBS BT_UUID_DECLARE_128(BT_UUID_LBS_VAL)
#define BT_UUID_LBS_BUTTON BT_UUID_DECLARE_128(BT_UUID_LBS_BUTTON_VAL)
#define BT_UUID_LBS_LED BT_UUID_DECLARE_128(BT_UUID_LBS_LED_VAL)
#define BT_UUID_SENSOR_SERVICE_VAL BT_UUID_128_ENCODE(0x12345678, 0x1234, 0x1234, 0x1234, 0x1234567890ab)
#define BT_UUID_SENSOR_DATA_VAL BT_UUID_128_ENCODE(0x12345678, 0x1234, 0x5678, 0x1234, 0x567812345679)

#define BT_UUID_SENSOR_SERVICE BT_UUID_DECLARE_128(BT_UUID_SENSOR_SERVICE_VAL)
#define BT_UUID_SENSOR_DATA BT_UUID_DECLARE_128(BT_UUID_SENSOR_DATA_VAL)


/** @brief Callback type for when an LED state change is received. */
typedef void (*led_cb_t)(const bool led_state);

/** @brief Callback type for when the button state is pulled. */
typedef bool (*button_cb_t)(void);

/** @brief Callback struct used by the LBS Service. */
struct my_lbs_cb {
	/** LED state change callback. */
	led_cb_t led_cb;
	/** Button read callback. */
	button_cb_t button_cb;
};

/** @brief Initialize the LBS Service.
 *
 * This function registers application callback functions with the My LBS
 * Service
 *
 * @param[in] callbacks Struct containing pointers to callback functions
 *			used by the service. This pointer can be NULL
 *			if no callback functions are defined.
 *
 *
 * @retval 0 If the operation was successful.
 *           Otherwise, a (negative) error code is returned.
 */
int my_lbs_init(struct my_lbs_cb *callbacks);

/** @brief Send the button state as indication.
 *
 * This function sends a binary state, typically the state of a
 * button, to all connected peers.
 *
 * @param[in] button_state The state of the button.
 *
 * @retval 0 If the operation was successful.
 *           Otherwise, a (negative) error code is returned.
 */
int my_lbs_send_button_state_indicate(bool button_state);

/** @brief Send the button state as notification.
 *
 * This function sends a binary state, typically the state of a
 * button, to all connected peers.
 *
 * @param[in] button_state The state of the button.
 *
 * @retval 0 If the operation was successful.
 *           Otherwise, a (negative) error code is returned.
 */
int my_lbs_send_button_state_notify(bool button_state);

/** @brief Send the sensor value as notification.
 *
 * This function sends an uint32_t  value, typically the value
 * of a simulated sensor to all connected peers.
 *
 * @param[in] sensor_value_x The value of the simulated sensor.
 * @param[in] sensor_value_y
 * @param[in] sensor_value_z
 * @retval 0 If the operation was successful.
 *           Otherwise, a (negative) error code is returned.
 */
int my_lbs_send_sensor_notify_xyz(int sensor_value_x, int sensor_value_y, int sensor_value_z);

int my_lbs_send_sensor_notify_x(int sensor_value_x);
int my_lbs_send_sensor_notify_y(int sensor_value_y);
int my_lbs_send_sensor_notify_z(int sensor_value_z);

#ifdef __cplusplus
}
#endif

/**
 * @}
 */

#endif /* BT_LBS_H_ */
