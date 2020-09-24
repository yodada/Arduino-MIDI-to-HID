/*
 *******************************************************************************
 * USB-MIDI dump utility
 * Copyright 2013-2015 Yuuichi Akagawa
 *
 * for use with USB Host Shield 2.0 from Circuitsathome.com
 * https://github.com/felis/USB_Host_Shield_2.0
 *
 * This is sample program. Do not expect perfect behavior.
 *******************************************************************************
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
 *******************************************************************************
 */

#include <usbh_midi.h>
#ifndef ARDUINO_SAM_DUE
#include <usbhub.h>
#endif

// Satisfy the IDE, which needs to see the include statment in the ino too.
#ifdef dobogusinclude
#include <spi4teensy3.h>
#include <SPI.h>
#endif

#define MIDI_DEBUG 0

USB  Usb;
//USBHub Hub(&Usb);
USBH_MIDI  Midi(&Usb);

void MIDI_poll();

boolean bFirst;
uint16_t pid, vid;

void setup()
{
  bFirst = true;
  vid = pid = 0;
  Serial.begin(115200);
  Serial1.begin(115200);
  delay( 200 );
}

void loop()
{
  unsigned long t1;

  Usb.Task();
  t1 = micros();
  if( Usb.getUsbTaskState() == USB_STATE_RUNNING )
  {
    MIDI_poll();
  }
  // put a small delay here
  delay( 100 );
}

// Poll USB MIDI Controler and send to serial MIDI
void MIDI_poll()
{
    char buf[20];
    uint8_t bufMidi[64];
    uint16_t  rcvd;
    uint8_t starter = 0xff;

    if(Midi.vid != vid || Midi.pid != pid){
      sprintf(buf, "VID:%04X, PID:%04X", Midi.vid, Midi.pid);
      Serial.println(buf);
      vid = Midi.vid;
      pid = Midi.pid;
    }
    if(Midi.RecvData( &rcvd,  bufMidi) == 0 ){
      // Print to serial if we are debugging
      if(MIDI_DEBUG) {
        sprintf(buf, "%08X: ", millis());
        Serial.print(buf);
        Serial.print(rcvd);
        Serial.print(':');
        for(int i=0; i<rcvd; i++){
          sprintf(buf, " %02X", bufMidi[i]);
          Serial.print(buf);
        }
        Serial.println("");
      }

      // All messages we use are 4 bytes long
      if (rcvd == 4) {
        Serial1.write(starter);
        Serial1.write(bufMidi[0]);
        Serial1.write(bufMidi[1]);
        Serial1.write(bufMidi[2]);
        Serial1.write(bufMidi[3]);
      }
   }
}
