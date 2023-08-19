#include "bitbang_I2C_controller.h"

#define MESSAGE 1

constexpr int MONITOR_PIN = 2;

void setup() {

#if MESSAGE
  Serial.begin(9600);
  while (!Serial)
    ;

  Serial.printf("\r***** Hello, bitbang_I2C_host! *****\n");
#endif  // MESSSAGE

  //  bbi2c_init(0, 1, 14);
  //  bbi2c_init(0, 1, 14);
  bbi2c_init(0, 1, 40);
  pinMode(MONITOR_PIN, OUTPUT);
  gpio_put(MONITOR_PIN, 1);
}

void loop() {
  ctrl_status err;

  gpio_put(MONITOR_PIN, 0);


  for (volatile int i = 0; i < 0; i++)
    ;

  uint8_t data[] = { 0x00 };
  err = write_transaction(0x9E, data, sizeof(data), false);

  if (err)
    Serial.printf("ERROR:%d\n", err);

  if (err == ARBITRATION_LOST) {
    Serial.printf("********************** ARBITRATION_LOST\n");
    delay(20);
  }

  gpio_put(MONITOR_PIN, 1);

  uint8_t tmp[2];
  err = read_transaction(0x90, tmp, sizeof(tmp));

#if MESSAGE
  Serial.printf("%f\n", ((int)(tmp[0]) << 8 | tmp[1]) / 256.0);

#endif  // MESSSAGE

  delay(100);
}