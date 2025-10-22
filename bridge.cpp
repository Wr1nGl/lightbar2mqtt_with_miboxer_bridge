#include "bridge.h"
#include "radio.h"
#include "remote.h"
#include <cmath>


void converter_ON_OFF(Radio *radio, Remote *remote, uint8_t value){
  radio->sendCommand(remote->getSerial(), 0x01);
  remote->callback(1, 0);
}

void converter_BRIGHTNESS(Radio *radio, Remote *remote, uint8_t value){
  radio->sendCommand(remote->getSerial(), 0x05, 0x0 - 16);
  radio->sendCommand(remote->getSerial(), 0x04, 0x0 + round(value/100.0 * 15.0));
}

void converter_TEMPERATURE_PRESS(Radio *radio, Remote *remote, uint8_t value){
  if (value == 0){
    radio->sendCommand(remote->getSerial(), 0x02, 0x00 + 3);
  }
  else{
    radio->sendCommand(remote->getSerial(), 0x03, 0x00 - 3);
  }
}

//can be set to different number if you want more precise control but i find it too slow...
void converter_TEMPERATURE_HOLD(Radio *radio, Remote *remote, uint8_t value){
  if (value == 0){
    radio->sendCommand(remote->getSerial(), 0x02, 0x00 + 3);
  }
  else{
    radio->sendCommand(remote->getSerial(), 0x03, 0x00 - 3);
  }
}
