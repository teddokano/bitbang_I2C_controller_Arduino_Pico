# bitbang_I2C_controller_Arduino_Pico
Bitbang I²C sample using Arduino-Pico. 

![write_and_read.png](https://github.com/teddokano/bitbang_I2C_controller_Arduino_Pico/blob/main/reference/pic/write_and_read.png)

Supporting multimaster.  
Next picture is showing the bitbang I²C controller is trying to access target address 0x9E with 400kHz bit clock. 
However, another controller started accedd to target 0x90 with 100kHz in same time.  
The I²C arbitration performed with clock synchronization. 
As the result of the arbitration, this bitbang I²C controller lost because another controller ws accessing to lower address (smaller address value) target. 
![multimaster-arbitration.png](https://github.com/teddokano/bitbang_I2C_controller_Arduino_Pico/blob/main/reference/pic/multimaster-arbitration.png)
