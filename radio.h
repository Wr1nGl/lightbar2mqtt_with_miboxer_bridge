#ifndef RADIO_H
#define RADIO_H

#include <RF24.h>
#include <CRC16.h>

#include "constants.h"
#include "remote.h"
#include "utils.h"

class Remote;

struct PackageIdForSerial
{
    uint32_t serial;
    boolean never_read = true;
    uint8_t package_id;
};

class Radio
{
public:
    Radio(uint8_t ce, uint8_t csn);
    ~Radio();
    void setup();
    void sendCommand(uint32_t serial, byte command, byte options);
    void sendCommand(uint32_t serial, byte command);
    void loop();
    bool addRemote(Remote *remote);
    bool removeRemote(Remote *remote);
    
private:
    RF24 radio;
    PackageIdForSerial package_ids[constants::MAX_SERIALS];
    uint8_t num_package_ids = 0;

    Remote *remotes[constants::MAX_REMOTES];
    uint8_t num_remotes = 0;

    void calculate_address();
    uint8_t receive_miboxer_address[5];
    const uint64_t receive_xiaomi_address = 0xAAAAAAAAAA;
    const uint64_t send_address = 0x5555555555;
    static constexpr byte preamble[8] = {0x53, 0x39, 0x14, 0xDD, 0x1C, 0x49, 0x34, 0x12};

    // For details on how these parameters were chosen, see
    // https://github.com/lamperez/xiaomi-lightbar-nrf24?tab=readme-ov-file#crc-checksum
    CRC16 crc = CRC16(0x1021, 0xfffe, 0x0000, false, false);
    
    uint16_t last_remote_packet_counter = 0xFFFF;
    uint16_t init_time = millis();
    boolean init_time_lock = false;
    void handle_xiaomi_Package();
    void handle_miboxer_Package();
    void set_miboxer_remote();
    void set_xiaomi_bar();
};

#endif