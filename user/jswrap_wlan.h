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

bool jswrap_wlan_scan(JsVar *wlanObj, JsVar *config, JsVar *callback);
//JsVar *jswrap_socket_connect(JsVar *spi, Pin cs, Pin en, Pin irq);
bool jswrap_wlan_connect(JsVar *wlanObj, JsVar *vAP, JsVar *vKey, JsVar *callback);
void jswrap_wlan_disconnect(JsVar *wlanObj);
void jswrap_wlan_reconnect(JsVar *wlanObj);
//JsVar *jswrap_wlan_getIP(JsVar *wlanObj);
//bool jswrap_wlan_setIP(JsVar *wlanObj, JsVar *options);

/// Check if the socket has disconnected (clears flag as soon as is called)
//bool socket_socket_has_closed(int socketNum);
