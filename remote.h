#ifndef REMOTE_H
#define REMOTE_H

#include "constants.h"
#include "radio.h"

class Radio;

class Remote
{
public:
    Remote(Radio *radio, uint32_t serial, const char *name, const uint8_t len_groups_ON, const uint8_t *trigger_groups_ON, const uint8_t len_groups_OFF, const uint8_t *trigger_groups_OFF, const uint8_t len_groups_DATA, const uint8_t *trigger_groups_DATA);
    ~Remote();

    uint32_t getSerial();
    String getSerialString();
    const char *getName();

    bool registerCommandListener(std::function<void(Remote *, byte, byte)> callback);
    bool unregisterCommandListener(std::function<void(Remote *, byte, byte)> callback);

    void callback(byte command, byte options);

    uint8_t* getTrigger_groups_ON();
    uint8_t* getTrigger_groups_OFF();
    uint8_t* getTrigger_groups_DATA();

    uint8_t getLen_trigger_groups_ON();
    uint8_t getLen_trigger_groups_OFF();
    uint8_t getLen_trigger_groups_DATA();

private:
    Radio *radio;
    uint32_t serial;
    const char *name;
    String serialString;

    std::function<void(Remote *, byte, byte)> commandListeners[constants::MAX_COMMAND_LISTENERS];
    uint8_t numCommandListeners = 0;

    void generate_trigger_groups(const uint8_t *trigger_groups, uint8_t *trigger_group_to_be_filled, int modifier, uint8_t length);
    uint8_t trigger_groups_ON[9];
    uint8_t trigger_groups_OFF[9];
    uint8_t trigger_groups_DATA[9];

    uint8_t len_trigger_groups_ON;
    uint8_t len_trigger_groups_OFF;
    uint8_t len_trigger_groups_DATA;
};

#endif