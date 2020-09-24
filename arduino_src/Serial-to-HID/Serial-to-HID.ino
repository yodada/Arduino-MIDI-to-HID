// This program read raw MIDI data from Serial1, decode,
// and map output as an HID
// Ground A0 to start reading from serial

#define MIDI_DEBUG 0
#include "Joystick.h"

// Create a Joystick instance
Joystick_ Joystick(JOYSTICK_DEFAULT_REPORT_ID, 
  JOYSTICK_TYPE_JOYSTICK, 32, 0,
  true, true, false, false, false, false,
  true, true, false, false, false);

void setup() {
  // Initialize serial ports
  Serial.begin(115200);
  Serial1.begin(115200);
  
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  
  // Initialize joystick
  Joystick.setXAxisRange(-127, 127);
  Joystick.setYAxisRange(-127, 127);
  Joystick.setThrottleRange(0, 127);
  Joystick.setRudderRange(-127, 127);
  Joystick.begin();

  // We use A0 to control the reading from serial
  // Ground A0 to start it
  pinMode(A0, INPUT_PULLUP);
  digitalWrite(LED_BUILTIN, LOW);
}

void loop() {

  // Global variables
  char buf[20];
  uint8_t pos = 0;
  uint8_t bufMidi[4];
  uint8_t starter = 0xff;
  
  // System Disabled
  if (digitalRead(A0) != 0)
  {
    // Turn indicator light off.
    digitalWrite(LED_BUILTIN, LOW);
    return;
  }

  
  if(Serial1.available()) {
    // Signal there is data
    digitalWrite(LED_BUILTIN, HIGH);
    
    // Keep reading garbage data until we find the start byte
    while(Serial1.available() && Serial1.read() != starter);

    // We should be able to read 4 data bytes
    pos = 0;
    while(pos < 4) {
      if(Serial1.available()) {
        bufMidi[pos] = Serial1.read();
        pos++;
      } else {
        delay(50);
      }
    }

    // Process MIDI data
    switch(bufMidi[0]) {
      // Button push, wait for 200ms, and release it
      case 0x09:
        if(bufMidi[1] == 0x9a && bufMidi[3] == 0x7f) {
          uint8_t button = bufMidi[2]+8;
          Joystick.pressButton(button);
          delay(100);
          Joystick.releaseButton(button);
        }
        break;
      // Knob turn
      case 0x0b:
        if(bufMidi[1] == 0xba) {
          // This is our throttle
          if (bufMidi[2] == 0x09) {
            uint8_t throttle = bufMidi[3];
            // Throttle value may not go back to 0 fully, so if value is < 10 we
            // assume it's 0
            if(throttle < 8) {
              throttle = 0;
            }
            if(throttle > 120) {
              throttle = 127;
            }
            Joystick.setThrottle(throttle);
          } else {
            uint8_t button = (bufMidi[2] - 1)*2;
            // Turn to the right
            if(bufMidi[3] > 0x40) {
              button += 1; // 0 to the left, 1 to the right
            }
            Joystick.pressButton(button);
            delay(50);
            Joystick.releaseButton(button);
          }
        }
        break;
    }

    if(MIDI_DEBUG) {
    sprintf(buf, "%08X: ", millis());
      Serial.print(buf);
      Serial.print(pos);
      Serial.print(':');
      for(int i=0; i<pos; i++){
        sprintf(buf, " %02X", bufMidi[i]);
        Serial.print(buf);
      }
      Serial.println("");
    }
  } else {
    digitalWrite(LED_BUILTIN, LOW);
  }
  
}
