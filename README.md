# ESP-control-with-UBUS
This is a demo of controlling ESP8266 devices over UBUS on the host machine. It requires UBUS and Ubox libraries built. The final package prepared for OpenWRT and accompanying the **tuyaespcontrol** will be added soon.

This demo program takes arguments formed as JSON messages from the terminal and it uses the UBUS system to interact with ESP devices connected to the host machine.

##Features
- Control ESP8266 devices: Control the GPIO pins of connected ESP microcontrollers via UBUS commands.
- Retrieve device data: Get sensor data (DHT sensors).
- Get devices connected: get only right (ESP8266) devices connected listed.

##Supported UBUS Methods
**devices** - returns a list of connected ESP devices with the following details:

- port: The serial port associated with the device.
- vendor id: The vendor ID of the ESP device.
- product id: The product ID of the ESP device.
- This allows for the detection of multiple ESP devices that can be connected to the router at the same time.

Usage:

```json
{"action": "devices"}
```

**on** - turns on the specified pin (sets it to HIGH).
Arguments:

- port: The serial port to which the ESP device is connected.
- pin: The pin number (valid pins range from 1 to 9).

Example:

```json
{"action": "on", "port": "/dev/ttyUSB0", "pin": 5}
```
**off** - turns off the specified pin (sets it to LOW).
Arguments:

- port: The serial port to which the ESP device is connected.
- pin: The pin number (valid pins range from 1 to 9).
Example:

```json
{"action": "off", "port": "/dev/ttyUSB0", "pin": 5}
```
**get** - retrieves data from attached sensors (only DHT sensors are supported for now).
Arguments:

- port: The serial port to which the ESP device is connected.
- pin: The pin number (valid pins range from 1 to 9).
- model: The sensor model (e.g., dht11).
- sensor: The type of sensor (currently only dht is supported).
Example:

```json
{"action": "get", "port": "/dev/ttyUSB0", "pin": 13, "model": "dht11", "sensor": "dht"}
```

In case of errors, UBUS will return an error message detailing the issue.
