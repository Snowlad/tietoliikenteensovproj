#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/gap.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/bluetooth/conn.h>
#include <dk_buttons_and_leds.h>
#include "my_lbs.h"
#include "adc.h"
#include "adc.c"

static struct bt_le_adv_param *adv_param = BT_LE_ADV_PARAM(
    (BT_LE_ADV_OPT_CONNECTABLE | BT_LE_ADV_OPT_USE_IDENTITY),
    800, 801, NULL);

LOG_MODULE_REGISTER(Lesson4_Exercise2, LOG_LEVEL_INF);

#define DEVICE_NAME CONFIG_BT_DEVICE_NAME
#define DEVICE_NAME_LEN (sizeof(DEVICE_NAME) - 1)

#define RUN_STATUS_LED DK_LED1
#define CON_STATUS_LED DK_LED2
#define USER_LED DK_LED3
#define USER_BUTTON DK_BTN1_MSK
#define USER_BUTTON_2 DK_BTN2_MSK

#define STACKSIZE 1024
#define PRIORITY 7

#define RUN_LED_BLINK_INTERVAL 1000
#define NOTIFY_INTERVAL 500

static bool app_button_state;
static bool button_2_pressed = false;

static const struct bt_data ad[] = {
    BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
    BT_DATA(BT_DATA_NAME_COMPLETE, DEVICE_NAME, DEVICE_NAME_LEN),
};

static const struct bt_data sd[] = {
    BT_DATA_BYTES(BT_DATA_UUID128_ALL, BT_UUID_LBS_VAL),
};

static void app_led_cb(bool led_state)
{
    dk_set_led(USER_LED, led_state);
}

static bool app_button_cb(void)
{
    return app_button_state;
}

void send_data_thread(void)
{
    int err = dk_leds_init();
    if (err) {
        LOG_ERR("LEDs init failed (err %d)\n", err);
        return;
    }

    if (initializeADC() != 0) {
        printk("ADC initialization failed!\n");
        return;
    }

    while (1) {
        if (button_2_pressed) {
            struct Measurement m = readADCValue();

            // Send all three values using the existing function
            err = my_lbs_send_sensor_notify_xyz(m.x, m.y, m.z);
            if (err) {
                printk("Failed to send notification (err %d)\n", err);
            } else {
                printk("Sent sensor data: x=%d, y=%d, z=%d\n", m.x, m.y, m.z);
            }
        }

        k_sleep(K_MSEC(NOTIFY_INTERVAL));
    }
}


/*void send_data_thread(void)
{
    int err;

    err = dk_leds_init();
    if (err) {
        LOG_ERR("LEDs init failed (err %d)\n", err);
        return;
    }

    if (initializeADC() != 0) {
        printk("ADC initialization failed!\n");
        return;
    }

    while (1) {
        if (button_2_pressed) {
            struct Measurement m = readADCValue();
            my_lbs_send_sensor_notify_x(m.x);
            my_lbs_send_sensor_notify_y(m.y);
            my_lbs_send_sensor_notify_z(m.z);
            printk("ADC values: x=%d, y=%d, z=%d\n", m.x, m.y, m.z);
        }

        k_sleep(K_MSEC(NOTIFY_INTERVAL));
    }
}*/

static struct my_lbs_cb app_callbacks = {
    .led_cb = app_led_cb,
    .button_cb = app_button_cb,
};

static void button_changed(uint32_t button_state, uint32_t has_changed)
{
    if (has_changed & USER_BUTTON) {
        uint32_t user_button_state = button_state & USER_BUTTON;
        my_lbs_send_button_state_indicate(user_button_state);
        app_button_state = user_button_state ? true : false;
    }

    if (has_changed & USER_BUTTON_2) {
        uint32_t user_button_2_state = button_state & USER_BUTTON_2;
        button_2_pressed = user_button_2_state ? true : false;
        printk("Button 2 %s\n", button_2_pressed ? "pressed" : "released");
    }
}

static void on_connected(struct bt_conn *conn, uint8_t err)
{
    if (err) {
        printk("Connection failed (err %u)\n", err);
        return;
    }

    printk("Connected\n");

    dk_set_led_on(CON_STATUS_LED);
}

static void on_disconnected(struct bt_conn *conn, uint8_t reason)
{
    printk("Disconnected (reason %u)\n", reason);

    dk_set_led_off(CON_STATUS_LED);
}

struct bt_conn_cb connection_callbacks = {
    .connected = on_connected,
    .disconnected = on_disconnected,
};

static int init_button(void)
{
    int err;

    err = dk_buttons_init(button_changed);
    if (err) {
        printk("Cannot init buttons (err: %d)\n", err);
    }

    return err;
}

int main(void)
{
    int blink_status = 0;
    int err;

    LOG_INF("Starting Lesson 4 - Exercise 2 \n");

    err = dk_leds_init();
    if (err) {
        LOG_ERR("LEDs init failed (err %d)\n", err);
        return -1;
    }

    err = init_button();
    if (err) {
        printk("Button init failed (err %d)\n", err);
        return -1;
    }

    err = bt_enable(NULL);
    if (err) {
        LOG_ERR("Bluetooth init failed (err %d)\n", err);
        return -1;
    }
    bt_conn_cb_register(&connection_callbacks);

    err = my_lbs_init(&app_callbacks);
    if (err) {
        printk("Failed to init LBS (err:%d)\n", err);
        return -1;
    }
    LOG_INF("Bluetooth initialized\n");
    err = bt_le_adv_start(adv_param, ad, ARRAY_SIZE(ad), sd, ARRAY_SIZE(sd));
    if (err) {
        LOG_ERR("Advertising failed to start (err %d)\n", err);
        return -1;
    }

    LOG_INF("Advertising successfully started\n");
    for (;;) {
        dk_set_led(RUN_STATUS_LED, (++blink_status) % 2);
        k_sleep(K_MSEC(RUN_LED_BLINK_INTERVAL));
    }
}

K_THREAD_DEFINE(send_data_thread_id, STACKSIZE, send_data_thread, NULL, NULL, NULL, PRIORITY, 0, 0);
