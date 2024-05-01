#include "config.h"
#include "reader.h"

void onCardRead(uint32_t);

void setup() {
  // onboard LED
  pinMode(2, OUTPUT);

  // Debug
  Serial.begin(115200);

  initReader(READER_PIN_D0, READER_PIN_D1, onCardRead);
}

void loop() {}

void onCardRead(uint32_t data) {
  Serial.print("Read card: ");
  Serial.println(data);
}