# Espruino-on-ESP8266
This is a temporary project. A goal is to run the Espruino JavaScript (github.com/espruino) built with the Xtensa under esp-open-sdk (github.com/pfalcon/esp-open-sdk) directly on ESP8266 wifi module.

All the code changes for now are "dirty", source files and headers are all in /user/. Once it pass the tests, we are going to contribute to Espruino repository.

# Recent achievements:
- Upload and run main.js (flash memory address 0x60000)
- Evaluate JS code from serial port (115200 baud) and save to flash.
- setTimeout & setInterval now work great (it came out as an issue - calling native functions with 1 or 2 arguments)
- GPIO functionality fully implemented. Analog read and write with PWM.

# To Do List:
There are few functionalities that need to be implemented as native javascript functions/objects before we can merge the code to Espruino:
- Incorporate a flash file system. We need to choose most suitable one (http://elm-chan.org/fsw/ff/00index_e.html or github.com/pellepl/spiffs)
- Socket + HTTP Web Server
- FTP protocol for easy web scripts uploading (html, css, js - client/server side)
- Web Socket as upgrade of HTTP protocol
- WiFi configuration
- Prepare python scripts for ESP8266 board for Espruino project

# Contributors are very welcome!
Currently progress is slow because I am the only developer and I work in regular leisure. There is a guy who helps a lot with testing, debugging, researching and consulting. Additional C/C++ developer would speed things up rapidly!

# How to run Espruino on ESP8266?
esptool.py --port /dev/ttyUSB0 write_flash 0x00000 firmware/0x00000.bin 0x10000 firmware/0x10000.bin

# How to upload & run JavaScript?
Customize your code in js/main.js and flash it:

esptool.py --port /dev/ttyUSB0 write_flash 0x60000 ./js/main.js

# Example JavaScript:
var gpio2 = new Pin(2);

gpio2.write(!gpio2.read()); // toggle LED GPIO2


setInterval(function() {

  var value = analogRead(); // read from photo sensor

  analogWrite(2, value); // output analog value to LED

}, 50);


save(); // write the code above to flash
