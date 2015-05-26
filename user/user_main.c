
#include "espmissingincludes.h"
#include "ets_sys.h"
#include "osapi.h"
#include "stdout.h"
#include "uart.h"
#include "user_interface.h"

#include "platform_config.h"
#include "jsinteractive.h"
#include "jshardware.h"

#include "jsvar.h"
#include "jswrap_functions.h"
#include "spi_flash.h"

#define FS_ADDRESS 0x60000

// error handler for pure virtual calls
//void __cxa_pure_virtual() { while (1); }

void jsInit(bool autoLoad) {
	jshInit();
	jsvInit();
	jsiInit(autoLoad);
}

void jsKill() {
	jsiKill();
	jsvKill();
	jshKill();
}

/*
 void vApplicationMallocFailedHook( void );
 void *pvPortMalloc( size_t xWantedSize );
 void vPortFree( void *pv );
 size_t xPortGetFreeHeapSize( void );
 void vPortInitialiseBlocks( void );
*/
/*void *alloca(size_t s) {
	void *p = os_malloc(s);
	os_printf("alloca %p, %d\n", p, s);
	return p;
}*/

#define malloc os_malloc
#define free os_free
//#define realloc os_realloc
/*void *malloc(size_t s) {
	void *p = os_malloc(s);
	os_printf("malloc %p, %d\n", p, s);
	return p;
}
void free(void *p) {
	os_printf("free %p, %d\n", p, sizeof(p));
	os_free(p);
}
void *os_realloc(void *old, size_t size) {
	size_t s = sizeof(old);
	if (size <= s) return old;
	void *new = os_malloc(size);
	//os_printf("realloc %p, %d, %p, %d\n", old, s, new, size);
	memcpy(new, old, s < size ? s : size);
	os_free(old);
	return new;
}*/

const char *ICACHE_RAM_ATTR jsVarToString(JsVar *jsVar) {
	if (!jsVar) return "undefined";
	jsVar = jsvAsString(jsVar, true/*unlock*/);
	size_t len = jsvGetStringLength(jsVar);
	if (0 == len) return "''";
	static char *str = NULL;
	static size_t size = 0;
	if (!str) str = malloc(size = len+32);
	else if (size < len+1) {
		free(str);
		str = malloc(size = len+32);
	}
	len = jsvGetString(jsVar, str, size);
	str[len] = 0;
	return str ? str : "undefined";
}

extern UartDevice UartDev;

void writeToFlash(JsVar *jsCode) {
	const char *code = jsVarToString(jsCode);
	int error;
	int addr = FS_ADDRESS;
	int sector = addr/SPI_FLASH_SEC_SIZE;
	int to = addr + strlen(code)+1;
	while (addr < to) {
		spi_flash_erase_sector(sector);
		if (SPI_FLASH_RESULT_OK != (error = spi_flash_write(addr, (uint32 *)code, SPI_FLASH_SEC_SIZE))) {
			jsiConsolePrintf("\nwriteToFlash error %d\n", error);
		}
		addr += SPI_FLASH_SEC_SIZE;
		code += SPI_FLASH_SEC_SIZE;
		sector++;
	}
}

#include "jswrapper.h"
void addNativeFunction(const char *name, void (*callbackPtr)(void)) {
	jsvUnLock(jsvObjectSetChild(execInfo.root, name, jsvNewNativeFunction(callbackPtr, JSWAT_VOID)));
}

static JsVar *jsUserCode = 0;
void nativeSave() {
	if (!jsUserCode) return;
	jsiConsolePrintf("nativeSave\n");
	writeToFlash(jsUserCode);
//	jsvUnLock(jsUserCode);
//	jsUserCode = 0;
}

void ICACHE_RAM_ATTR user_init(void) {
//	return;
//	ets_delay_us(1000);

	uart_init(BIT_RATE_115200, 0);
//	os_printf("Heap size: %d\n", system_get_free_heap_size());
	
//	ets_delay_us(1000);
	
	jsInit(true);
	addNativeFunction("save", nativeSave);
//	addNativeFunction("discard", nativeDiscard);

	//testFunctionCall();
	system_print_meminfo();

	jsiConsolePrintf("\nReady\n");
//	os_printf("Heap size: %d\n", system_get_free_heap_size());

	//runTimer();

	JsVar *jsCode = 0;
	jsiConsolePrintf("\nRead from flash:\n");
	char c;
	int error;
	int addr;
	for (addr = FS_ADDRESS;; addr++) {
		if (SPI_FLASH_RESULT_OK != (error = spi_flash_read(addr, (uint32 *)&c, 1))) {
			jsiConsolePrintf("\nerror %d\n", error);
			jsvUnLock(jsCode);
			jsCode = 0;
			break;
		}
		if (0x80 & c || 0 == c) break; // allow ascii only
		if (!jsCode) jsCode = jsvNewFromEmptyString();
		jsvAppendStringBuf(jsCode, &c, 1);
		uart0_putc(c);
	}
	if (jsCode) {
		JsVar *jsResult = jspEvaluateVar(jsCode, 0, true);
		jsvUnLock(jsCode); jsCode = 0;
		if (jsResult) {
			jsiConsolePrintf("%v\n", jsResult);
			jsvUnLock(jsResult);
		}
	}
//	os_printf("Heap size: %d\n", system_get_free_heap_size());

	bool cr = false;
	while (true) {
		while ((c = uart_getc())) {
			uart0_putc(c);
			if (cr && '\n' == c) {
				if (jsCode) {
//					writeToFlash(jsCode);
					JsVar *jsResult = jspEvaluateVar(jsCode, 0, true);
					jsvUnLock(jsCode); jsCode = 0;
					jsiConsolePrintf("%v\n", jsResult);
					jsvUnLock(jsResult);
				}
			} else if (0 < c && !(cr = '\r' == c)) {
				if (!jsCode) jsCode = jsvNewFromEmptyString();
				jsvAppendStringBuf(jsCode, &c, 1);
				if (!jsUserCode) jsUserCode = jsvLockAgain(jsCode);
				if (jsUserCode != jsCode) jsvAppendStringBuf(jsUserCode, &c, 1);
			}
		}
//		os_printf("Heap size: %d\n", system_get_free_heap_size());
		jsiLoop();
	}
	jsKill();
}

