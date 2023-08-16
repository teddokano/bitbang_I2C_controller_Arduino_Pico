#include "bitbang_I2C_controller.h"

#define   MESSAGE   0

void setup() {

#if MESSAGE
  Serial.begin(9600);
  while (!Serial)
    ;

  Serial.println("\r***** Hello, bitbang_I2C_host! *****");
#endif  // MESSSAGE

  bbi2c_init(0, 1, 30);
}

void loop() {
  uint8_t data[] = { 0x00, 0xAA, 0x55 };
  write_transaction(0x90, data, sizeof(data), false);

  uint8_t tmp[2];
  read_transaction(0x92, tmp, sizeof(tmp));

#if MESSAGE
  Serial.println(((int)(tmp[0]) << 8 | tmp[1]) / 256.0);
#endif  // MESSSAGE

  delay(1);
}