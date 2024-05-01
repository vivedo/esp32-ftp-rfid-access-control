#include "reader.h"

Wiegand wiegand;

void readerTask(void*);
void pinStateChanged();
void receivedData(uint8_t*, uint8_t, void (*)(uint32_t));

#ifdef READER_DEBUG
void stateChanged(bool, void*);
void receivedDataError(Wiegand::DataError, uint8_t*, uint8_t, void*);

const char* DEBUG_PREFIX = "[READER] ";
#endif

uint8_t PIN_D0 = 0;
uint8_t PIN_D1 = 0;

void initReader(uint8_t _PIN_D0, uint8_t _PIN_D1, void (*onCardRead)(uint32_t)) {
  PIN_D0 = _PIN_D0;
  PIN_D1 = _PIN_D1;

  xTaskCreate(readerTask, "reader", 1024, (void*)onCardRead, 10, NULL);
}

void readerTask(void* pvParameters) {
  pinMode(PIN_D0, INPUT);
  pinMode(PIN_D1, INPUT);

  wiegand.onReceive(receivedData, (void (*)(uint32_t))pvParameters);

#ifdef READER_DEBUG
  wiegand.onReceiveError(receivedDataError, (void*)NULL);
  wiegand.onStateChange(stateChanged, (void*)NULL);
#endif

  wiegand.begin(Wiegand::LENGTH_ANY, true);

  attachInterrupt(digitalPinToInterrupt(PIN_D0), pinStateChanged, CHANGE);
  attachInterrupt(digitalPinToInterrupt(PIN_D1), pinStateChanged, CHANGE);

#ifdef READER_DEBUG
  Serial.print(DEBUG_PREFIX);
  Serial.println("Initialized");
#endif

  pinStateChanged();

  for (;;) {
    noInterrupts();
    wiegand.flush();
    interrupts();

    delay(100);
  }
}

// When any of the pins have changed, update the state of the wiegand library
void pinStateChanged() {
  wiegand.setPin0State(digitalRead(PIN_D0));
  wiegand.setPin1State(digitalRead(PIN_D1));
}

// Notifies when a card was read
void receivedData(uint8_t* data, uint8_t bits, void (*onCardRead)(uint32_t)) {
  uint8_t bytes = (bits + 7) / 8;

  uint32_t value = 0;
  for (uint8_t i = 0; i < bytes; i++) {
    value |= (data[i] & 0xFF) << ((bytes - i - 1) * 8);
  }

#ifdef READER_DEBUG
  Serial.print(DEBUG_PREFIX);
  Serial.print("Read ");
  Serial.print(bits);
  Serial.print("bits card / 0x");
  Serial.print(value, HEX);
  Serial.print(" / ");
  Serial.println(value);
#endif

  onCardRead(value);
}

#ifdef READER_DEBUG

// Notifies when a reader has been connected or disconnected
void stateChanged(bool plugged, void* params) {
  Serial.print(DEBUG_PREFIX);
  Serial.println(plugged ? "Connected" : "Disconnected");
}

// Notifies when an invalid transmission is detected
void receivedDataError(Wiegand::DataError error, uint8_t* rawData, uint8_t rawBits, void* params) {
  Serial.print(DEBUG_PREFIX);
  Serial.print(Wiegand::DataErrorStr(error));
  Serial.print(" - Raw data: ");
  Serial.print(rawBits);
  Serial.print("bits / ");

  //Print value in HEX
  uint8_t bytes = (rawBits + 7) / 8;
  for (int i = 0; i < bytes; i++) {
    Serial.print(rawData[i] >> 4, 16);
    Serial.print(rawData[i] & 0xF, 16);
  }
  Serial.println();
}

#endif