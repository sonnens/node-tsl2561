# htu21d

node interface for the TSL2561 luminosity sensor

## Install
````bash
$ npm install tsl2561
````

## Usage

```javascript
var tsl2561 = require('tsl2561');

var device = "/dev/i2c-1"
var address = 0x39;

var sensor = new tsl2561.Tsl2561(device, address);

var luminosity = sensor.lux();

````

## Raspberry Pi Setup


````bash
$ sudo vi /etc/modules
````

Add these two lines

````bash
i2c-bcm2708 
i2c-dev
````

````bash
$ sudo vi /etc/modprobe.d/raspi-blacklist.conf
````

Comment out blacklist i2c-bcm2708

````
#blacklist i2c-bcm2708
````

Load kernel module

````bash
$ sudo modprobe i2c-bcm2708

````

## Questions?

http://www.twitter.com/johnnysunshine

