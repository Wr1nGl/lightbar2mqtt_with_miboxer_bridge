#pragma once
#include <stdint.h>
#include <stddef.h>
#include <Arduino.h>

class Remote; 
class Radio;

typedef void (*CommandFunction)(Radio *radio, Remote *remote, uint8_t value);

struct CONVERTER{
  uint16_t miboxer;
  uint8_t packet_group_data_location;
  CommandFunction function;
};

void converter_ON_OFF(Radio *radio, Remote *remote, uint8_t value);
void converter_BRIGHTNESS(Radio *radio, Remote *remote, uint8_t value);
void converter_TEMPERATURE_PRESS(Radio *radio, Remote *remote, uint8_t value);
void converter_TEMPERATURE_HOLD(Radio *radio, Remote *remote, uint8_t value);

const CONVERTER converted_commands[5] = {
  {0x01, 5, converter_ON_OFF},
  {0x05, 7, converter_BRIGHTNESS},
  {0x24, 7, converter_TEMPERATURE_PRESS},
  {0xA4, 7, converter_TEMPERATURE_HOLD}
};