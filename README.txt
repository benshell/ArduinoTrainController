Arduino/Bluetooth/Android DC Train Controller
Developed by Benjamin Shell (Github: benshell, Twitter: @benshell)

This is a DC train controller, with advanced analog features for triggering the bell and whistle of a QSI decoder. The hardware that goes along with this is simply:

1) Arduino Uno
2) MegaMoto shield: http://www.robotpower.com/products/MegaMoto_info.html
3) Bluetooth JY-MCU board, as found on eBay/etc for about $8, wired to the Arduino with the serial TX/RX pins
4) A 27V Meanwell power supply
5) A buck converter (from eBay) to convert the 27V down to a voltage suitable for Arduino (so there's only one power connection for everything)
6) An Android phone running an app called BlueScripts, which connects to the Bluetooth board and sends serial commands

A sample BlueScripts.xml file for BlueScripts is included in the repository, but the Bluetooth device ID for each board will be different so this file will need to be updated.

I use this system outdoors for a garden railway, and achieve a Bluetooth range of 30 to 50 feet. Typically I need to be within 20 or 30 feet to connect, but once connected the operating range is a little better. Your mileage may vary.
