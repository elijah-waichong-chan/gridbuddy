#pragma once

#include <esp_now.h>
#include <WiFi.h>
#include "global_vars.h"

void wireless_Init(void);
void SendMessage(void);