#pragma once
#include <stdint.h>
#include <stddef.h>
#include <Arduino.h>

#define V2_OFFSET(byte, key, jumpStart) ( \
  V2_OFFSETS[byte-1][key%4] \
    + \
  ((jumpStart > 0 && key >= jumpStart && key < jumpStart+0x80) ? 0x80 : 0) \
)
#define V2_OFFSET_JUMP_START 0x54

static uint8_t const V2_OFFSETS[][4] = {
  { 0x45, 0x1F, 0x14, 0x5C }, // request type
  { 0x2B, 0xC9, 0xE3, 0x11 }, // id 1
  { 0x6D, 0x5F, 0x8A, 0x2B }, // id 2
  { 0xAF, 0x03, 0x1D, 0xF3 }, // command
  { 0x1A, 0xE2, 0xF0, 0xD1 }, // argument
  { 0x04, 0xD8, 0x71, 0x42 }, // sequence
  { 0xAF, 0x04, 0xDD, 0x07 }, // group
  { 0x61, 0x13, 0x38, 0x64 }  // checksum
};

uint8_t xorKey(uint8_t key);
uint8_t reverseBits(uint8_t byte);
boolean validate_miboxer_crc(uint8_t *packet, size_t length);
void decodeV2Packet(uint8_t *packet);
uint8_t decodeByte(uint8_t byte, uint8_t s1, uint8_t xorKey, uint8_t s2);
void format(uint8_t const* packet, char* buffer);