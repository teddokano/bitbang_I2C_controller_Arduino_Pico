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

  bbi2c_init(0, 1, 400 * 1000);  //  SDA_pin#, SCL_pin#, frequency[Hz]
  additional_io_pins(4, 3);

  pinMode(MONITOR_PIN, OUTPUT);
  gpio_put(MONITOR_PIN, 1);

//  force_set_bbi2c_WAIT_VAL( 0 );
}

void loop() {
  uint8_t data[] = { 0x00 };
  ctrl_status err;

#if 0
  gpio_put(MONITOR_PIN, 0);

  for (volatile int i = 0; i < 300; i++)
    ;

  err = write_transaction(0x9E, data, sizeof(data), false);

  if (err)
    Serial.printf("ERROR:%d\n", err);

  if (err == ARBITRATION_LOST) {
    Serial.printf("********************** ARBITRATION_LOST\n");
    delay(1);
  }

  gpio_put(MONITOR_PIN, 1);
#endif

  uint8_t tmp[2];
  err = write_transaction(0x90, data, sizeof(data), true);
  err = read_transaction(0x90, tmp, sizeof(tmp));

#if MESSAGE
  Serial.printf("temperature = %.3f [deg-C]\n", ((int)(tmp[0]) << 8 | tmp[1]) / 256.0);

#endif  // MESSSAGE

  delay(100);
}