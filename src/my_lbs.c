/*
 * Copyright (c) 2018 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

/** @file
 *  @brief LED Button Service (LBS) sample
 */

#include <zephyr/types.h>
#include <stddef.h>
#include <string.h>
#include <errno.h>
#include <zephyr/sys/printk.h>
#include <zephyr/sys/byteorder.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/bluetooth/gatt.h>

#include "my_lbs.h"

LOG_MODULE_DECLARE(Lesson4_Exercise2);

static bool notify_mysensor_enabled;
static bool indicate_enabled;
static bool button_state;
static struct my_lbs_cb lbs_cb;

/* STEP 4 - Define an indication parameter */
static struct bt_gatt_indicate_params ind_params;

/* STEP 3 - Implement the configuration change callback function */
static void mylbsbc_ccc_cfg_changed(const struct bt_gatt_attr *attr, uint16_t value)
{
	indicate_enabled = (value == BT_GATT_CCC_INDICATE);
}

/* STEP 13 - Define the configuration change callback function for the MYSENSOR characteristic */
/*static void mylbsbc_ccc_mysensor_cfg_changed(const struct bt_gatt_attr *attr, uint16_t value)
{
	notify_mysensor_enabled = (value == BT_GATT_CCC_NOTIFY);
}*/

// This function is called when a remote device has acknowledged the indication at its host layer
static void indicate_cb(struct bt_conn *conn, struct bt_gatt_indicate_params *params, uint8_t err)
{
	LOG_DBG("Indication %s\n", err != 0U ? "fail" : "success");
}
static ssize_t write_led(struct bt_conn *conn, const struct bt_gatt_attr *attr, const void *buf,
			 uint16_t len, uint16_t offset, uint8_t flags)
{
	LOG_DBG("Attribute write, handle: %u, conn: %p", attr->handle, (void *)conn);

	if (len != 1U) {
		LOG_DBG("Write led: Incorrect data length");
		return BT_GATT_ERR(BT_ATT_ERR_INVALID_ATTRIBUTE_LEN);
	}

	if (offset != 0) {
		LOG_DBG("Write led: Incorrect data offset");
		return BT_GATT_ERR(BT_ATT_ERR_INVALID_OFFSET);
	}

	if (lbs_cb.led_cb) {
		// Read the received value
		uint8_t val = *((uint8_t *)buf);

		if (val == 0x00 || val == 0x01) {
			// Call the application callback function to update the LED state
			lbs_cb.led_cb(val ? true : false);
		} else {
			LOG_DBG("Write led: Incorrect value");
			return BT_GATT_ERR(BT_ATT_ERR_VALUE_NOT_ALLOWED);
		}
	}

	return len;
}


static void sensor_ccc_cfg_changed(const struct bt_gatt_attr *attr, uint16_t value)
{
    if (value == BT_GATT_CCC_NOTIFY) {
        printk("Sensor data notifications enabled\n");
        notify_mysensor_enabled = true;
    } else {
        printk("Sensor data notifications disabled\n");
        notify_mysensor_enabled = false;
    }
}





static ssize_t read_button(struct bt_conn *conn, const struct bt_gatt_attr *attr, void *buf,
			   uint16_t len, uint16_t offset)
{
	// get a pointer to button_state which is passed in the BT_GATT_CHARACTERISTIC() and stored in attr->user_data
	const char *value = attr->user_data;

	LOG_DBG("Attribute read, handle: %u, conn: %p", attr->handle, (void *)conn);

	if (lbs_cb.button_cb) {
		// Call the application callback function to update the get the current value of the button
		button_state = lbs_cb.button_cb();
		return bt_gatt_attr_read(conn, attr, buf, len, offset, value, sizeof(*value));
	}

	return 0;
}

/* LED Button Service Declaration */
// First Service Definition
// LBS Service Definition
BT_GATT_SERVICE_DEFINE(
    my_lbs_svc,
    BT_GATT_PRIMARY_SERVICE(BT_UUID_LBS),

    BT_GATT_CHARACTERISTIC(BT_UUID_LBS_BUTTON, BT_GATT_CHRC_READ | BT_GATT_CHRC_INDICATE,
                           BT_GATT_PERM_READ, read_button, NULL, &button_state),
    BT_GATT_CCC(mylbsbc_ccc_cfg_changed, BT_GATT_PERM_READ | BT_GATT_PERM_WRITE),

    BT_GATT_CHARACTERISTIC(BT_UUID_LBS_LED, BT_GATT_CHRC_WRITE,
                           BT_GATT_PERM_WRITE, NULL, write_led, NULL)
);

// Sensor Service Definition
// Define the Service and Characteristic for Sensor Data
BT_GATT_SERVICE_DEFINE(
    sensor_svc,
    BT_GATT_PRIMARY_SERVICE(BT_UUID_SENSOR_SERVICE),

    BT_GATT_CHARACTERISTIC(BT_UUID_SENSOR_DATA, BT_GATT_CHRC_NOTIFY,  // Enable Notify
                           BT_GATT_PERM_NONE, 
                           NULL, NULL, NULL), // Placeholder for read, write functions

    // Client Characteristic Configuration Descriptor (for enabling notifications)
    BT_GATT_CCC(sensor_ccc_cfg_changed, BT_GATT_PERM_READ | BT_GATT_PERM_WRITE)
);







/* A function to register application callbacks for the LED and Button characteristics  */
int my_lbs_init(struct my_lbs_cb *callbacks)
{
	if (callbacks) {
		lbs_cb.led_cb = callbacks->led_cb;
		lbs_cb.button_cb = callbacks->button_cb;
	}

	return 0;
}

/* STEP 5 - Define the function to send indications */
int my_lbs_send_button_state_indicate(bool button_state)
{
	if (!indicate_enabled) {
		return -EACCES;
	}
	ind_params.attr = &my_lbs_svc.attrs[2];
	ind_params.func = indicate_cb; // A remote device has ACKed at its host layer (ATT ACK)
	ind_params.destroy = NULL;
	ind_params.data = &button_state;
	ind_params.len = sizeof(button_state);
	return bt_gatt_indicate(NULL, &ind_params);
}

int my_lbs_send_sensor_notify_xyz(int sensor_value_x, int sensor_value_y, int sensor_value_z)
{
    // Check if notifications are enabled
    if (!notify_mysensor_enabled) {
        printk("Notifications are not enabled\n");
        return -EACCES; // Return access error if notifications are not enabled
    }

    // Combine all sensor values into a buffer (each 16-bit)
    int16_t sensor_data[3] = {sensor_value_x, sensor_value_y, sensor_value_z};  // 3 sensor values

    // Ensure the correct attribute index (0 for sensor_data characteristic in sensor_svc)
    if (sensor_svc.attrs[0].read == NULL) {
        printk("Invalid attribute index for sensor data characteristic\n");
        return -EINVAL; // Return invalid argument error if attribute index is incorrect
    }

    // Send the combined data as a single notification (6 bytes for 3 int16_t values)
    return bt_gatt_notify(NULL, &sensor_svc.attrs[2], sensor_data, sizeof(sensor_data));

}







/* STEP 14 - Define the function to send notifications for the MYSENSOR characteristic */

/*int my_lbs_send_sensor_notify_x(int sensor_value_x)
{
	if (!notify_mysensor_enabled) {
		return -EACCES;
	}

	return bt_gatt_notify(NULL, &my_lbs_svc.attrs[7], &sensor_value_x, sizeof(sensor_value_x));
}*/

/*int my_lbs_send_sensor_notify_y(int sensor_value_y)
{
	if (!notify_mysensor_enabled) {
		return -EACCES;
	}

	return bt_gatt_notify(NULL, &my_lbs_svc.attrs[7], &sensor_value_y, sizeof(sensor_value_y));
}

int my_lbs_send_sensor_notify_z(int sensor_value_z)
{
	if (!notify_mysensor_enabled) {
		return -EACCES;
	}

	return bt_gatt_notify(NULL, &my_lbs_svc.attrs[7], &sensor_value_z, sizeof(sensor_value_z));
}
*/