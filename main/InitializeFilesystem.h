/*
 * SPDX-FileCopyrightText: 2023 Brian Pugh
 * SPDX-FileCopyrightText: 2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include "esp_err.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_littlefs.h"
#include "queues.h"
#include "messages.h"

void InitializeFilesystem(void);

extern led_actuation_t led_actuation_order;