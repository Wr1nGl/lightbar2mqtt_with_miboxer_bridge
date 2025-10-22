#include "radio.h"
#include "config.h"
#include "bridge.h"

/*
 * Package structure fo xiaomi bar:
 *  0 –  7: Preamble (see constant above)
 *  8 – 10: Remote ID
 * 11 – 11: Separator (0xFF)
 * 12 – 12: Sequence counter
 * 13 – 14: Command ID + options
 * 15 – 16: CRC16 checksum
 */

Radio::Radio(uint8_t ce, uint8_t csn)
{
    this->radio = RF24(ce, csn);
}

Radio::~Radio()
{
    this->radio.stopListening();
    this->radio.powerDown();
}

bool Radio::addRemote(Remote *remote)
{
    if (this->num_remotes >= constants::MAX_REMOTES)
    {
        Serial.println("[Radio] Could not add remote, because too many remotes are saved!");
        Serial.println("[Radio] Please check if you actually want to save more than " + String(constants::MAX_REMOTES, DEC) + " remotes.");
        Serial.println("[Radio] If you do, increase MAX_REMOTES in constants.h and recompile.");
        return false;
    }
    if (this->num_package_ids >= constants::MAX_SERIALS)
    {
        Serial.println("[Radio] Could not add remote, because too many serials are saved!");
        Serial.println("[Radio] Please check if you actually want to save more than " + String(constants::MAX_SERIALS, DEC) + " serials.");
        Serial.println("[Radio] If you do, increase MAX_SERIALS in constants.h and recompile.");
        return false;
    }
    this->remotes[this->num_remotes] = remote;
    this->num_remotes++;
    this->package_ids[this->num_package_ids].serial = remote->getSerial();
    this->package_ids[this->num_package_ids].package_id = 0;
    this->num_package_ids++;
    Serial.print("[Radio] Remote ");
    Serial.print(remote->getSerialString());
    Serial.println(" added!");
    return true;
}

bool Radio::removeRemote(Remote *remote)
{
    for (int i = 0; i < this->num_remotes; i++)
    {
        if (this->remotes[i] == remote)
        {
            for (int j = 0; j < this->num_package_ids; j++)
            {
                if (this->package_ids[j].serial == remote->getSerial())
                {
                    for (int k = j; k < this->num_package_ids - 1; k++)
                    {
                        this->package_ids[k] = this->package_ids[k + 1];
                    }
                    this->num_package_ids--;
                    break;
                }
            }

            for (int j = i; j < this->num_remotes - 1; j++)
            {
                this->remotes[j] = this->remotes[j + 1];
            }
            this->num_remotes--;
            return true;
        }
    }
    return false;
}

void Radio::calculate_address(){
    uint8_t *syncwordBytes = this->receive_miboxer_address;
    //5 byte address is expected, others wont work
    int ix = 5;
    syncwordBytes[ --ix ] = reverseBits(
        ((MIBOXER_FIRST_ADDRESS_PART << 4) & 0xF0) | (MIBOXER_PREAMBLE & 0x0F)
    );
    syncwordBytes[ --ix ] = reverseBits((MIBOXER_FIRST_ADDRESS_PART >> 4) & 0xFF);
    syncwordBytes[ --ix ] = reverseBits(((MIBOXER_FIRST_ADDRESS_PART >> 12) & 0x0F) + ((MIBOXER_SECOND_ADDRESS_PART << 4) & 0xF0));
    syncwordBytes[ --ix ] = reverseBits((MIBOXER_SECOND_ADDRESS_PART >> 4) & 0xFF);
    syncwordBytes[ --ix ] = reverseBits(
        ((MIBOXER_SECOND_ADDRESS_PART >> 12) & 0x0F) | ((MIBOXER_TRAILER << 4) & 0xF0)
    );
}

void Radio::sendCommand(uint32_t serial, byte command, byte options)
{
    //if we are sending command always send on xiaomi radio setting
    this->set_xiaomi_bar();
    PackageIdForSerial *package_id = nullptr;
    for (int i = 0; i < this->num_package_ids; i++)
    {
        if (this->package_ids[i].serial == serial)
        {
            package_id = &this->package_ids[i];
            break;
        }
    }
    if (package_id == nullptr)
    {
        if (this->num_package_ids >= constants::MAX_SERIALS)
        {
            Serial.println("[Radio] Could not send command, because too many serials are saved!");
            Serial.println("[Radio] Please check if you actually want to save more than " + String(constants::MAX_SERIALS, DEC) + " serials.");
            Serial.println("[Radio] If you do, increase MAX_SERIALS in constants.h and recompile.");
            return;
        }
        package_id = &this->package_ids[this->num_package_ids];
        package_id->serial = serial;
        package_id->package_id = 0;
        this->num_package_ids++;
    }
    
    byte data[17] = {0};
    memcpy(data, Radio::preamble, sizeof(Radio::preamble));
    data[8] = (serial & 0xFF0000) >> 16;
    data[9] = (serial & 0x00FF00) >> 8;
    data[10] = serial & 0x0000FF;
    data[11] = 0xFF;
    data[12] = ++package_id->package_id;
    data[13] = command;
    data[14] = options;

    this->crc.restart();
    this->crc.add(data, sizeof(data) - 2);
    uint16_t checksum = this->crc.calc();
    data[15] = (checksum & 0xFF00) >> 8;
    data[16] = checksum & 0x00FF;

    Serial.print("[Radio] Sending command: 0x");
    for (int i = 0; i < 17; i++)
    {
        Serial.print(data[i], HEX);
    }
    Serial.println();

    this->radio.stopListening();
    for (int i = 0; i < 20; i++)
    {
        this->radio.write(&data, sizeof(data), true);
        delay(10);
    }
    this->set_miboxer_remote();
}

void Radio::sendCommand(uint32_t serial, byte command)
{
    return this->sendCommand(serial, command, 0x0);
}

void Radio::set_miboxer_remote(){
    Serial.println("[Radio] Setting miboxer remote settings");
    this->radio.stopListening();
    this->radio.setChannel(MIBOXER_RADIO_CHANNEL + 2);
    this->radio.setDataRate(RF24_1MBPS);
    //data+1(packet_length)+2(crc)
    this->radio.setPayloadSize(MIBOXER_PACKET_LENGTH + 1 + 2);
    this->radio.startListening();
}

void Radio::set_xiaomi_bar(){
    Serial.println("[Radio] Setting xiaomi bar settings");
    this->radio.stopListening();
    this->radio.setChannel(68);
    this->radio.setPayloadSize(17);
    this->radio.setDataRate(RF24_2MBPS);
}

void Radio::setup()
{
    uint retries = 0;
    while (!this->radio.begin())
    {
        Serial.println("[Radio] nRF24 not responding! Is it wired correctly?");
        delay(1000);
        retries++;
        if (retries > 60)
            ESP.restart();
    }

    if (WAIT_TIME_ON_STARTUP == 0){
        this->init_time_lock = true;
    }

    //calculate the miboxer remote address, in-place change
    this->calculate_address();

    Serial.println("[Radio] Setting up radio...");
    this->radio.failureDetected = false;

    if (this->init_time_lock){
        this->radio.setChannel(MIBOXER_RADIO_CHANNEL + 2);
        this->radio.setDataRate(RF24_1MBPS);
        //data+1(packet_length)+2(crc)
        this->radio.setPayloadSize(MIBOXER_PACKET_LENGTH + 1 + 2);
    }
    else{
        this->radio.setChannel(68);
        this->radio.setDataRate(RF24_2MBPS);
        //data+1(packet_length)+2(crc)
        this->radio.setPayloadSize(17);
    }
    this->radio.openReadingPipe(0, Radio::receive_miboxer_address);
    this->radio.openReadingPipe(1, Radio::receive_xiaomi_address);
    this->radio.disableCRC();
    this->radio.disableDynamicPayloads();
    this->radio.setAutoAck(false);
    this->radio.setRetries(0, 0);
    this->radio.openWritingPipe(Radio::send_address);

    this->radio.startListening();
    Serial.println("[Radio] done!");
}

void Radio::loop()
{
    if (this->radio.failureDetected)
    {
        Serial.println("[Radio] Failure detected!");
        delay(1000);
        this->setup();
        delay(1000);
    }

    uint8_t pipe_number;
    if (this->radio.available(&pipe_number)){
        if (pipe_number == 0) {
            this->handle_miboxer_Package();
        }
        else if (pipe_number == 1) {
            handle_xiaomi_Package();
        }

        if (!this->init_time_lock && millis() - this->init_time > WAIT_TIME_ON_STARTUP * 1000) {
            this->radio.closeReadingPipe(1);
            this->set_miboxer_remote();
            this->init_time_lock = true;
        }
    }    
}

void Radio::handle_xiaomi_Package()
{
    // Read raw data, append a 5 and shift it. See
    // https://github.com/lamperez/xiaomi-lightbar-nrf24?tab=readme-ov-file#baseband-packet-format
    // on why that is necessary.
    byte raw_data[18] = {0};
    this->radio.read(&raw_data, sizeof(raw_data));
    byte data[17] = {0x5};
    for (int i = 0; i < 17; i++)
    {
        if (i == 0)
            data[i] = 0x50 | raw_data[i] >> 5;
        else
            data[i] = ((raw_data[i - 1] >> 1) & 0x0F) << 4 | ((raw_data[i - 1] & 0x01) << 3) | raw_data[i] >> 5;
    }

    // Check if preamble matches. Ignore package otherwise.
    if (memcmp(data, Radio::preamble, sizeof(Radio::preamble)))
        return;

    // Make sure the checksum of the package is correct.
    this->crc.restart();
    this->crc.add(data, sizeof(data) - 2);
    uint16_t calculated_checksum = this->crc.calc();
    uint16_t package_checksum = data[15] << 8 | data[16];
    if (calculated_checksum != package_checksum)
    {
        Serial.println("[Radio] Ignoring pacakge with wrong checksum!");
        return;
    }

    // Check if package is coming from a observed remote.
    Remote *remote = nullptr;
    uint32_t serial = data[8] << 16 | data[9] << 8 | data[10];
    for (int i = 0; i < this->num_remotes; i++)
    {

        if (serial == this->remotes[i]->getSerial())
        {
            remote = this->remotes[i];
            break;
        }
    }

    if (remote == nullptr)
    {
        Serial.print("[Radio] Ignoring package with unknown serial: 0x");
        Serial.print(serial, HEX);
        Serial.println("");
        return;
    }

    // Make sure the same package was not handled before.
    uint8_t package_id = data[12];
    PackageIdForSerial *package_id_for_serial = nullptr;
    for (int i = 0; i < this->num_package_ids; i++)
    {
        if (this->package_ids[i].serial == serial)
        {
            package_id_for_serial = &this->package_ids[i];
            break;
        }
    }
    if (package_id_for_serial == nullptr)
    {
        Serial.print("[Radio] Could not find latest package id for serial 0x");
        Serial.print(serial, HEX);
        Serial.println("!");
        return;
    }
    if (package_id <= package_id_for_serial->serial && package_id > package_id_for_serial->serial - 64)
    {
        Serial.println("[Radio] Ignoring package with too low package number!");
        return;
    }

    if (package_id_for_serial->never_read) {
        package_id_for_serial->never_read = false;
    }
    else if (package_id_for_serial->package_id == package_id){
        return; 
    }
    
    package_id_for_serial->package_id = package_id;

    Serial.println("[Radio] Package received!");
    remote->callback(data[13], data[14]);
}

void Radio::handle_miboxer_Package()
{
    //data+1(packet_length)+2(crc)
    int packet_length = MIBOXER_PACKET_LENGTH + 1 + 2;
    uint8_t packet[packet_length];
    int outp = 0;

    radio.read(packet, packet_length);

    for (int inp = 0; inp < packet_length; inp++) {
        packet[outp++] = reverseBits(packet[inp]);
    }

    if (outp < 2) {
        return;
    }

    //miboxer crc validation
    if (!validate_miboxer_crc(packet, outp - 2)) {
        return;
    }

    outp -= 2;
        
    uint8_t packet_without_crc_and_padding[MIBOXER_PACKET_LENGTH];
    memcpy(packet_without_crc_and_padding, packet + 1, MIBOXER_PACKET_LENGTH);
    
    //good for reading packet format received by the remote, only works on newer, scrabmbled packets (as this entire repository, lol)
    /*
    char array[200];
    format(packet_without_crc_and_padding, array);
    Serial.println(array);
    //*/

    //in-place change
    decodeV2Packet(packet_without_crc_and_padding);
    
    if (this->last_remote_packet_counter != packet_without_crc_and_padding[6]){
        //set last counter to stop receiving multiple commands with the same counter
        this->last_remote_packet_counter = 0;
        this->last_remote_packet_counter = packet_without_crc_and_padding[6];

        //loop through the command config
        for (const auto& converter : converted_commands){
            if (converter.miboxer == packet_without_crc_and_padding[4]){
                Remote *remote = nullptr;
                //loop through all the remotes
                for (int i = 0; i < this->num_remotes; i++){
                    remote = this->remotes[i];
                    //loop all the trigger channels
                    boolean found_command = false;
                    //if this is removed, data will trigger on/off functions
                    if (packet_without_crc_and_padding[4] == 0x01){
                        for (int q = 0; q < remote->getLen_trigger_groups_ON(); q++){
                            //check if the given group is valid for given remote
                            if (packet_without_crc_and_padding[converter.packet_group_data_location] == remote->getTrigger_groups_ON()[q]){
                                /*
                                Serial.println("Activating ON");
                                Serial.println(packet_without_crc_and_padding[converter.packet_group_data_location]);
                                Serial.println(remote->getTrigger_groups_ON()[q]);
                                //*/
                                //send signal to turn the bar on or off for all the remotes
                                converter.function(this, remote, packet_without_crc_and_padding[5]);
                                found_command = true;
                                break;
                            }
                        }
                    
                        //if command was found no need to continue the loop the off button allowed commands
                        if (found_command)
                            continue;

                        for (int q = 0; q < remote->getLen_trigger_groups_OFF(); q++){
                            //check if the given group is valid for given remote
                            if (packet_without_crc_and_padding[converter.packet_group_data_location] == remote->getTrigger_groups_OFF()[q]){
                                /*
                                Serial.println("Activating OFF");
                                Serial.println(packet_without_crc_and_padding[converter.packet_group_data_location]);
                                Serial.println(remote->getTrigger_groups_OFF()[q]);
                                //*/
                                //send signal to turn the bar on or off for all the remotes
                                converter.function(this, remote, packet_without_crc_and_padding[5]);
                                found_command = true;
                                break;
                            }
                        }
                    }
                    else{
                        for (int q = 0; q < remote->getLen_trigger_groups_DATA(); q++){
                            //check if the given group is valid for given remote
                            if (packet_without_crc_and_padding[converter.packet_group_data_location] == remote->getTrigger_groups_DATA()[q]){
                                /*
                                Serial.println("Activating DATA");
                                Serial.println(packet_without_crc_and_padding[converter.packet_group_data_location]);
                                Serial.println(remote->getTrigger_groups_DATA()[q]);
                                //*/
                                //send signal to turn the bar on or off for all the remotes
                                converter.function(this, remote, packet_without_crc_and_padding[5]);
                                found_command = true;
                                break;
                            }
                        }
                    }
                }
                break;
            }
        }
    }
}