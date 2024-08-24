#pragma once

#include <Arduino.h>
#include <driver/gpio.h>
#include <driver/twai.h>

int can_calls_timeout = pdMS_TO_TICKS(10);

uint32_t enabledCanAlerts =
    TWAI_ALERT_ERR_ACTIVE |
    TWAI_ALERT_RECOVERY_IN_PROGRESS |
    TWAI_ALERT_BUS_RECOVERED |
    TWAI_ALERT_ABOVE_ERR_WARN |
    TWAI_ALERT_BUS_ERROR |
    TWAI_ALERT_RX_QUEUE_FULL |
    TWAI_ALERT_ERR_PASS |
    TWAI_ALERT_BUS_OFF |
    TWAI_ALERT_RX_FIFO_OVERRUN |
    TWAI_ALERT_TX_RETRIED |
    TWAI_ALERT_PERIPH_RESET;

void canStart(int txPin, int rxPin) {

    Serial.println("Starting CAN Connection");
    // twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT((gpio_num_t)txPin, (gpio_num_t)rxPin, TWAI_MODE_NORMAL);
    twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT((gpio_num_t)txPin, (gpio_num_t)rxPin, TWAI_MODE_NO_ACK);
    twai_timing_config_t t_config = TWAI_TIMING_CONFIG_1MBITS();
    twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

    esp_err_t err;
    if (err = twai_driver_install(&g_config, &t_config, &f_config) == ESP_OK) {
        Serial.println("CAN Driver installed");
    } else {
        Serial.printf("ERROR: Failed to install CAN driver - %s\n", esp_err_to_name(err));
        return;
    }

    if (err = twai_start() == ESP_OK) {
        Serial.println("CAN Driver started");
    } else {
        Serial.printf("ERROR: Failed to start CAN driver - %s\n", esp_err_to_name(err));
        return;
    }

    if (err = twai_reconfigure_alerts(enabledCanAlerts, NULL) == ESP_OK) {
        Serial.println("CAN Alerts reconfigured");
    } else {
        Serial.printf("ERROR: Failed to reconfigure alerts - %s\n", esp_err_to_name(err));
    }
}

void canSend(twai_message_t message) {
    message.extd = 1;              // Standard vs extended format
    message.rtr = 0;               // Data vs RTR frame
    message.ss = 0;                // Whether the message is single shot (i.e. does not repeat on error)
    message.self = 1;              // Whether the message is a self reception request (loopback)
    message.dlc_non_comp = 0;      // DLC is less than 8

    esp_err_t err;
    if (err = twai_transmit(&message, can_calls_timeout) == ESP_OK) {
        // Serial.println("CAN message queued for transmission");
    } else {
        Serial.printf("CAN failed to queue message for transmission - %s\n", esp_err_to_name(err));
    }
}

bool canRecieve() {
    twai_message_t rxMessage;
    if (twai_receive(&rxMessage, can_calls_timeout) != ESP_OK) {
        return false;
    }

    // Serial.printf("Recieved CAN frame: %02X#", rxMessage.identifier);
    // if (!(rxMessage.rtr)) {
    //     for (int i = 0; i < rxMessage.data_length_code; i++) {
    //         Serial.printf("%02X", rxMessage.data[i]);
    //     }
    // }
    // Serial.println();
    return true;
}

void getCanAlerts() {
    uint32_t alerts;
    twai_read_alerts(&alerts, can_calls_timeout);
    
    if (alerts & enabledCanAlerts == 0) return;
    if (alerts & TWAI_ALERT_ERR_ACTIVE != 0) { Serial.println("CAN Alert: TWAI_ALERT_ERR_ACTIVE"); }
    if (alerts & TWAI_ALERT_RECOVERY_IN_PROGRESS != 0) { Serial.println("CAN Alert: TWAI_ALERT_RECOVERY_IN_PROGRESS"); }
    if (alerts & TWAI_ALERT_BUS_RECOVERED != 0) { Serial.println("CAN Alert: TWAI_ALERT_BUS_RECOVERED"); }
    if (alerts & TWAI_ALERT_ABOVE_ERR_WARN != 0) { Serial.println("CAN Alert: TWAI_ALERT_ABOVE_ERR_WARN"); }
    if (alerts & TWAI_ALERT_BUS_ERROR != 0) { Serial.println("CAN Alert: TWAI_ALERT_BUS_ERROR"); }
    if (alerts & TWAI_ALERT_RX_QUEUE_FULL != 0) { Serial.println("CAN Alert: TWAI_ALERT_RX_QUEUE_FULL"); }
    if (alerts & TWAI_ALERT_ERR_PASS != 0) { Serial.println("CAN Alert: TWAI_ALERT_ERR_PASS"); }
    if (alerts & TWAI_ALERT_BUS_OFF != 0) { Serial.println("CAN Alert: TWAI_ALERT_BUS_OFF"); }
    if (alerts & TWAI_ALERT_RX_FIFO_OVERRUN != 0) { Serial.println("CAN Alert: TWAI_ALERT_RX_FIFO_OVERRUN"); }
    if (alerts & TWAI_ALERT_TX_RETRIED != 0) { Serial.println("CAN Alert: TWAI_ALERT_TX_RETRIED"); }
    if (alerts & TWAI_ALERT_PERIPH_RESET != 0) { Serial.println("CAN Alert: TWAI_ALERT_PERIPH_RESET"); }
}

void getCanStatus() {
    twai_status_info_t status;
    twai_get_status_info(&status);

    Serial.println("CAN Controller Status: ");
    Serial.print("    state - ");
    switch(status.state) {
        case TWAI_STATE_STOPPED: Serial.print("STOPPED\n"); break;
        case TWAI_STATE_RUNNING: Serial.print("RUNNING\n"); break;
        case TWAI_STATE_BUS_OFF: Serial.print("BUS_OFF\n"); break;
        case TWAI_STATE_RECOVERING: Serial.print("RECOVERING\n"); break;
    }
    Serial.printf("    msgs_to_tx - %i\n", status.msgs_to_tx);
    Serial.printf("    msgs_to_rx - %i\n", status.msgs_to_rx);
    Serial.printf("    tx_error_counter - %i\n", status.tx_error_counter);
    Serial.printf("    rx_error_counter - %i\n", status.rx_error_counter);
    Serial.printf("    tx_failed_count - %i\n", status.tx_failed_count);
    Serial.printf("    rx_missed_count - %i\n", status.rx_missed_count);
    Serial.printf("    rx_overrun_count - %i\n", status.rx_overrun_count);
    Serial.printf("    arb_lost_count - %i\n", status.arb_lost_count);
    Serial.printf("    bus_error_count - %i\n", status.bus_error_count);
}
