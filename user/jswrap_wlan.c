/*
 * This file is part of Espruino, a JavaScript interpreter for Microcontrollers
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * ----------------------------------------------------------------------------
 * This file is designed to be parsed during the build process
 *
 * Contains built-in functions for Espressif ESP8266 WiFi Access
 * ----------------------------------------------------------------------------
 */

#include "jswrap_wlan.h"
#include "jshardware.h"
#include "jsinteractive.h"

#include "user_interface.h"

#define WLAN_OBJ_NAME "wlan"
#define WLAN_ON_STATE_CHANGE "#onstate"
#define WLAN_ON_SCAN "#onscan"

#define WLAN_SCAN_SSID "ssid"
#define WLAN_SCAN_BSSID "bssid"
#define WLAN_SCAN_CHANNEL "channel"
#define WLAN_SCAN_SHOW_HIDDEN "showHidden"

//#include "network.h"
//#include "network_esp8266.h"

/*JSON{
  "type" : "class",
  "class" : "Wifi"
}
An instantiation of an ESP8266 network adaptor
*/

/*JSON{
  "type" : "staticmethod",
  "class" : "Wifi",
  "name" : "connect",
  "generate" : "jswrap_wlan_connect",
  "params" : [
    ["ap","JsVar","Access point name"],
    ["key","JsVar","WPA2 key (or undefined for unsecured connection)"],
    ["callback","JsVar","Function to call back with connection status. It has one argument which is one of 'connect'/'disconnect'/'dhcp'"]
  ],
  "return" : ["bool",""]
}
Connect to an access point
*/


// WIFI
//uint8 wifi_get_opmode();
//bool wifi_set_opmode(uint8 opmode);
//bool wifi_station_get_config(struct *config);
//bool wifi_station_set_config(struct *config);
//bool wifi_station_connect();

bool jswrap_wlan_connect(JsVar *wlanObj, JsVar *vAP, JsVar *vKey, JsVar *callback) {
	if (!(jsvIsUndefined(callback) || jsvIsFunction(callback))) {
		jsError("Expecting callback Function but got %t", callback);
		return false;
	}
	
	if (jsvIsFunction(callback)) {
		jsvObjectSetChild(wlanObj, WLAN_ON_STATE_CHANGE, callback);
	}
	
	jsvObjectSetChild(wlanObj, JS_HIDDEN_CHAR_STR"AP", vAP); // no unlock intended
	jsvObjectSetChild(wlanObj, JS_HIDDEN_CHAR_STR"KEY", vKey); // no unlock intended

	uint8 opcode = wifi_get_opmode();
	if (!(STATION_MODE & opcode || wifi_set_opmode(STATION_MODE | opcode)))
		return false;
	
	jsiConsolePrintf("1");
	struct station_config config;
	jsvGetString(vAP, config.ssid, sizeof(config.ssid));
	jsvGetString(vKey, config.password, sizeof(config.password));
	config.bssid_set = 0;
	
	jsiConsolePrintf("2");
	//wifi_station_get_config
	if (STATION_IDLE != wifi_station_get_connect_status())
		wifi_station_disconnect();
	jsiConsolePrintf("3");
	bool ret = wifi_station_set_config(&config) && wifi_station_connect();
	JsVar *data = jsvNewFromString(ret ? "OK" :"FAIL");
	jsiQueueObjectCallbacks(wlanObj, WLAN_ON_STATE_CHANGE, &data, 1);
	return ret;
}

//bool wifi_station_disconnect();
void jswrap_wlan_disconnect(JsVar *wlanObj) {
	wifi_station_disconnect();
}

void jswrap_wlan_reconnect(JsVar *wlanObj) {
  JsVar *ap = jsvObjectGetChild(wlanObj, JS_HIDDEN_CHAR_STR"AP", 0);
  JsVar *key = jsvObjectGetChild(wlanObj, JS_HIDDEN_CHAR_STR"KEY", 0);
  JsVar *cb = jsvObjectGetChild(wlanObj, WLAN_ON_STATE_CHANGE, 0);
  jswrap_wlan_connect(wlanObj, ap, key, cb);
  jsvUnLock(ap);
  jsvUnLock(key);
  jsvUnLock(cb);
}

//uint8 wifi_station_get_connect_status();
//bool wifi_station_scan(struct scan_config *config, scan_done_cb_t cb);
////void scan_done_cb_t(void *arg, STATUS status);
//static JsVar *wifi_scan_callback;
void on_wlan_scan_done(void *arg, STATUS status) {
//	if (!wifi_scan_callback) return;
	// call wifi_scan_callback();
//	JsVar jsArg = jsvNewFromString("");
/*
	STAILQ_ENTRY(bss_info)     next;
	uint8 bssid[6];
	uint8 ssid[32];
	uint8 channel;
	sint8 rssi;
	AUTH_MODE authmode;
	uint8 is_hidden;
*/
	struct bss_info *a = (struct bss_info *)arg;
	while (a) {
		jsiConsolePrintf("%s", a->ssid);
		a++;
	}
	JsVar *wlanObj = jsvObjectGetChild(execInfo.hiddenRoot, WLAN_OBJ_NAME, 0);
	JsVar *data = jsvNewFromString("OK");
	jsiQueueObjectCallbacks(wlanObj, WLAN_ON_SCAN, &data, 1);
//	jsvUnLock(wifi_scan_callback);
//	wifi_scan_callback = 0;
}

bool jswrap_wlan_scan(JsVar *wlanObj, JsVar *config, JsVar *callback) {
	JsVar *ssid = jsvObjectGetChild(config, WLAN_SCAN_SSID, 0);
	JsVar *bssid = jsvObjectGetChild(config, WLAN_SCAN_BSSID, 0);
	JsVar *channel = jsvObjectGetChild(config, WLAN_SCAN_CHANNEL, 0);
	JsVar *showHidden = jsvObjectGetChild(config, WLAN_SCAN_SHOW_HIDDEN, 0);
	
	struct scan_config congig = {0};
	if (jsvIsString(ssid)) jsvGetString(ssid, (congig.ssid = alloca(32)), 32);
	if (jsvIsString(bssid)) jsvGetString(bssid, (congig.bssid = alloca(32)), 32);
	if (jsvIsNumeric(channel)) congig.channel = jsvGetInteger(channel);
	if (jsvIsNumeric(showHidden)) congig.show_hidden = jsvGetInteger(showHidden);
	
	if (jsvIsFunction(callback)) {
		jsvObjectSetChild(wlanObj, WLAN_ON_SCAN, callback); // no unlock needed
	}
	else if (!jsvIsUndefined(callback)) {
		jsError("Expecting callback Function but got %t", callback);
		return false;
	}
	return wifi_station_scan(NULL, on_wlan_scan_done); //&config
}

//bool wifi_station_ap_number_set(uint8 number);
//uint8 wifi_station_get_ap_info(struct config config[]);
//bool wifi_station_ap_change(uint8 current_ap_id);
//uint8 wifi_station_get_current_ap_id();
//uint8 wifi_station_get_auto_connect();
//bool wifi_station_set_auto_connect(uint8 set);
//bool wifi_station_dhcpc_start();
//bool wifi_station_dhcpc_stop();
//uint8 dhcp_status wifi_station_dhcpc_status();
//bool wifi_softap_get_config(struct config *config);
//bool wifi_softap_set_config(struct softap_config *config);
//struct station_info *wifi_softap_get_info();
//void wifi_softap_free_station_info();
//bool wifi_softap_dhcps_start();
//bool wifi_softap_dhcps_stop();
//bool wifi_softap_set_dhcps_lease(struct lease *please);
//uint8 dhcp_status wifi_softap_dhcps_status();
//bool wifi_set_phy_mode(uint8 mode mode);
//uint8 phy_mode wifi_get_mode();
//bool wifi_get_ip_info(uint8 if_index, struct info *info);

//uint8bool wifi_set_ip_info(uint8 if_index, struct ip_info *info);
//bool wifi_set_macaddr(uint8 if_index, *macaddr);
//bool wifi_get_macaddr(uint8 if_index , *macaddr);
//bool wifi_set_sleep_type(uint8 sleep_type type);
//uint8 wifi_get_type();
//void wifi_status_led_install(uint8 gpio_id, uint32 name, uint8 gpio_func);
//void wifi_status_led_uninstall();
//bool wifi_set_broadcast_if(uint8 interface);
//uint8 wifi_get_broadcast_if();



static JsVar *smartconfig_start_callback = 0;
void on_smartconfig_start(void *data) {
//	call smartconfig_start_callback();
}
//SmartConfig
/*
//bool smartconfig_start(sc_type type, callback_t cb, ...);
bool jswrap_smartconfig_start(JsVar *type, JsVar *callback) {
	if (callback) smartconfig_start_callback = jsvLockAgain(callback);
	return smartconfig_start(type, smartconfig_start_callback ? on_smartconfig_start : NULL);
}

//bool smartconfig_stop();
bool jswrap_smartconfig_stop() {
	return smartconfig_stop();
}

//sc_status get_smartconfig_status(void);
JsVar *jswrap_smartconfig_getStatus() {
	return get_smartconfig_status();
}
*/


