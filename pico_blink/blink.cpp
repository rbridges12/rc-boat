/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include "pico/stdlib.h"
#include "LoRa.hpp"

constexpr int LED_DELAY_MS = 250;
constexpr int MOSI_PIN = 3;
constexpr int MISO_PIN = 4;
constexpr int CS_PIN = 5;
constexpr int SCK_PIN = 2;
constexpr int RESET_PIN = 6;
constexpr int DIO0_PIN = 7;

LoRa lora(MISO_PIN, MOSI_PIN, SCK_PIN, CS_PIN, RESET_PIN, DIO0_PIN);

void LoRa::onDio0Rise(uint gpio, uint32_t events)
{
  lora.handleDio0Rise();
}

void onReceive(int packetSize) {
    printf("received message\n");
}

int main() {
    stdio_init_all();
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
    sleep_ms(1000);
    printf("initialized led\n");

    if (!lora.begin(915E6)) {
        printf("LoRa init failed\n");
        while (true);
    }
    printf("LoRa begin succeeded\n");

    lora.onReceive(onReceive);
    lora.receive();
    printf("LoRa init succeeded\n");

    while (true) {
        // printf("Hello, blink!\n");
        gpio_put(PICO_DEFAULT_LED_PIN, true);
        sleep_ms(LED_DELAY_MS);
        gpio_put(PICO_DEFAULT_LED_PIN, false);
        sleep_ms(LED_DELAY_MS);
    }
}
