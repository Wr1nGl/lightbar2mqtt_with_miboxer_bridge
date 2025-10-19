#ifndef REMOTE_H
#define REMOTE_H

#include "constants.h"
#include "radio.h"

class Radio;

class Remote
{
public:
    Remote(Radio *radio, uint32_t serial, const char *name, uint8_t trigger_groups_len, const uint8_t *trigger_groups);
    ~Remote();

    uint32_t getSerial();
    String getSerialString();
    const char *getName();

    bool registerCommandListener(std::function<void(Remote *, byte, byte)> callback);
    bool unregisterCommandListener(std::function<void(Remote *, byte, byte)> callback);

    void callback(byte command, byte options);

    uint8_t getNum_triggers();
    uint8_t* getTrigger_groups();

private:
    Radio *radio;
    uint32_t serial;
    const char *name;
    String serialString;

    std::function<void(Remote *, byte, byte)> commandListeners[constants::MAX_COMMAND_LISTENERS];
    uint8_t numCommandListeners = 0;

    void generate_trigger_groups(const uint8_t *trigger_groups);
    uint8_t trigger_groups[18];
    uint8_t num_triggers = 0;
};

#endif