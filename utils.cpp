#include <utils.h>
#include "config.h"

uint8_t xorKey(uint8_t key) {
  // Generate most significant nibble
  const uint8_t shift = (key & 0x0F) < 0x04 ? 0 : 1;
  const uint8_t x = (((key & 0xF0) >> 4) + shift + 6) % 8;
  const uint8_t msn = (((4 + x) ^ 1) & 0x0F) << 4;

  // Generate least significant nibble
  const uint8_t lsn = ((((key & 0xF) + 4)^2) & 0x0F);

  return ( msn | lsn );
}

void decodeV2Packet(uint8_t *packet) {
  uint8_t key = xorKey(packet[0]);

  for (size_t i = 1; i <= 8; i++) {
    packet[i] = decodeByte(packet[i], 0, key, V2_OFFSET(i, packet[0], V2_OFFSET_JUMP_START));
  }
}

uint8_t decodeByte(uint8_t byte, uint8_t s1, uint8_t xorKey, uint8_t s2) {
  uint8_t value = byte - s2;
  value = value ^ xorKey;
  value = value - s1;

  return value;
}

#define CRC_POLY 0x8408
boolean validate_miboxer_crc(uint8_t *data, size_t data_length) {
    uint16_t recvCrc = (data[data_length + 1] << 8) | data[data_length];
    uint16_t state = 0;
    for (size_t i = 0; i < data_length; i++) {
        uint8_t byte = data[i];
        for (int j = 0; j < 8; j++) {
        if ((byte ^ state) & 0x01) {
            state = (state >> 1) ^ CRC_POLY;
        } else {
            state = state >> 1;
        }
        byte = byte >> 1;
        }
    }

    if (state != recvCrc)
        return false;

    return true;
}

uint8_t reverseBits(uint8_t byte) {
  uint8_t result = byte;
  uint8_t i = 7;

  for (byte >>= 1; byte; byte >>= 1) {
    result <<= 1;
    result |= byte & 1;
    --i;
  }

  return result << i;
}

void format(uint8_t const* packet, char* buffer) {
    buffer += sprintf_P(buffer, PSTR("Raw packet: "));
    for (size_t i = 0; i < MIBOXER_PACKET_LENGTH; i++) {
        buffer += sprintf_P(buffer, PSTR("%02X "), packet[i]);
    }

    uint8_t decodedPacket[MIBOXER_PACKET_LENGTH];
    memcpy(decodedPacket, packet, MIBOXER_PACKET_LENGTH);

    decodeV2Packet(decodedPacket);

    buffer += sprintf_P(buffer, PSTR("\n\nDecoded:\n"));
    buffer += sprintf_P(buffer, PSTR("Key      : %02X\n"), decodedPacket[0]);
    buffer += sprintf_P(buffer, PSTR("b1       : %02X\n"), decodedPacket[1]);
    buffer += sprintf_P(buffer, PSTR("ID       : %02X%02X\n"), decodedPacket[2], decodedPacket[3]);
    buffer += sprintf_P(buffer, PSTR("Command  : %02X\n"), decodedPacket[4]);
    buffer += sprintf_P(buffer, PSTR("Argument : %02X\n"), decodedPacket[5]);
    buffer += sprintf_P(buffer, PSTR("Sequence : %02X\n"), decodedPacket[6]);
    buffer += sprintf_P(buffer, PSTR("Group    : %02X\n"), decodedPacket[7]);
    buffer += sprintf_P(buffer, PSTR("Checksum : %02X"), decodedPacket[8]);
}