#include "remote.h"
#include "remote.h"
Remote::Remote(Radio *radio, uint32_t serial, const char *name, const uint8_t len_groups_ON, const uint8_t *trigger_groups_ON, const uint8_t len_groups_OFF, const uint8_t *trigger_groups_OFF, const uint8_t len_groups_DATA, const uint8_t *trigger_groups_DATA)
{
    this->radio = radio;
    this->serial = serial;
    this->name = name;

    this->radio->addRemote(this);

    this->serialString = "0x" + String(this->serial, HEX);

    this->len_trigger_groups_ON = len_groups_ON;
    this->len_trigger_groups_OFF = len_groups_OFF;
    this->len_trigger_groups_DATA = len_groups_DATA;
    
    this->generate_trigger_groups(trigger_groups_ON, this->trigger_groups_ON, 0, len_groups_ON);
    this->generate_trigger_groups(trigger_groups_OFF, this->trigger_groups_OFF, 9, len_groups_OFF);
    this->generate_trigger_groups(trigger_groups_DATA, this->trigger_groups_DATA, 0, len_groups_DATA);
}

Remote::~Remote()
{
}

uint8_t* Remote::getTrigger_groups_ON(){
    return this->trigger_groups_ON;
}

uint8_t Remote::getLen_trigger_groups_ON(){
    return this->len_trigger_groups_ON;
}

uint8_t* Remote::getTrigger_groups_OFF(){
    return this->trigger_groups_OFF;
}

uint8_t Remote::getLen_trigger_groups_OFF(){
    return this->len_trigger_groups_OFF;
}

uint8_t* Remote::getTrigger_groups_DATA(){
    return this->trigger_groups_DATA;
}

uint8_t Remote::getLen_trigger_groups_DATA(){
    return this->len_trigger_groups_DATA;
}

void Remote::generate_trigger_groups(const uint8_t *trigger_groups, uint8_t *trigger_group_to_be_filled, int modifier, uint8_t length){
    for (int i = 0; i < length; i++){
        trigger_group_to_be_filled[i] = trigger_groups[i] + modifier;
    }
}

uint32_t Remote::getSerial()
{
    return this->serial;
}

String Remote::getSerialString()
{
    return this->serialString;
}

const char *Remote::getName()
{
    return this->name;
}

void Remote::callback(byte command, byte options)
{
    for (int i = 0; i < this->numCommandListeners; i++)
    {
        this->commandListeners[i](this, command, options);
    }
}

bool Remote::registerCommandListener(std::function<void(Remote *, byte, byte)> callback)
{
    if (this->numCommandListeners >= constants::MAX_COMMAND_LISTENERS)
    {
        Serial.println("[Remote] Could not add command listener to remote, because too many are saved!");
        Serial.println("[Remote] Please check if you actually want to save more than " + String(constants::MAX_COMMAND_LISTENERS, DEC) + " command listeners.");
        Serial.println("[Remote] If you do, increase MAX_COMMAND_LISTENERS in constants.h and recompile.");
        return false;
    }
    this->commandListeners[this->numCommandListeners] = callback;
    this->numCommandListeners++;
    return true;
}

bool Remote::unregisterCommandListener(std::function<void(Remote *, byte, byte)> callback)
{
    for (int i = 0; i < this->numCommandListeners; i++)
    {
        if (this->commandListeners[i].target<void(Remote *, byte, byte)>() == callback.target<void(Remote *, byte, byte)>())
        {
            for (int j = i; j < this->numCommandListeners - 1; j++)
            {
                this->commandListeners[j] = this->commandListeners[j + 1];
            }
            this->numCommandListeners--;
            return true;
        }
    }
    return false;
}