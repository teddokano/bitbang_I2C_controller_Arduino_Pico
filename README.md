# bitbang_I2C_controller_Arduino_Pico

## What is this?
Bitbang I²C sample using Arduino-Pico. This code has been made to explain how an I²C controller to can be implemanted.  
![write_and_read.png](https://github.com/teddokano/bitbang_I2C_controller_Arduino_Pico/blob/main/reference/pic/write_and_read.png)

> **Note**  
> This code is written to use a Raspberry Pi Pico with [Arduino-Pico](https://github.com/earlephilhower/arduino-pico)
> To setup the Arduino IDE, follow instruction of "Installing via Arduino Boards Manager" section in `README.md` of Arduino-Pico repository.  

> **Note**  
> To get best performance, choose optimization seting of "-O2" or "-O3".  
> This optimization setting gives ability of **1MHz I²C** comminication.   
> Even if the clock speed is set to 1MHz, the controller may not enough to drive the I²C bus as "Fast-mode Plus" because The Raspberry Pi Pico's single pin current drive capability is limited to 12mA.  
To increase the current, consider to use a I²C buffer (like PCA9617, PCA9600, etc) or use 2 pins for each SDA and SCL to drive 20mA on those signals.  
![optimization_setting.png](https://github.com/teddokano/bitbang_I2C_controller_Arduino_Pico/blob/main/reference/pic/optimization_setting.png)
_Choosing a optimization option_

## Features
### Basic features
This bitbang I²C controller supports basic operations of I²C bus.  
It can read/write data from/into target devices.  
A NACK response from target can be handled as an error which returned from API function. 

### Extended feature
#### Clock stretching
This controller supports clock stretching. 

#### Supporting multi-controller (a.k.a multi-master)  
Next picture is showing the bitbang I²C controller is trying to access target address 0x9E with 400kHz bit clock. 
However, another controller started accedd to target 0x90 with 100kHz in same time.  
The I²C arbitration performed with clock synchronization. 
As the result of the arbitration, this bitbang I²C controller lost because another controller ws accessing to lower address (smaller address value) target. 
![multimaster-arbitration.png](https://github.com/teddokano/bitbang_I2C_controller_Arduino_Pico/blob/main/reference/pic/multimaster-arbitration.png)
