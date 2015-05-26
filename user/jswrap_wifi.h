/*
 * This file is part of Espruino, a JavaScript interpreter for Microcontrollers
 *
 * Copyright (C) 2013 Gordon Williams <gw@pur3.co.uk>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * ----------------------------------------------------------------------------
 * Contains built-in functions for Espressif ESP8266 WiFi Access
 * ----------------------------------------------------------------------------
 */
#include "jsvar.h"

bool jswrap_wifi_scan(JsVar *config, JsVar *callback);
bool jswrap_wifi_connect(JsVar *vSSID, JsVar *vPassword); //, JsVar *callback
bool jswrap_wifi_disconnect();
