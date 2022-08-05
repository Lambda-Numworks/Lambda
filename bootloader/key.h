#ifndef BOOTLOADER_KEY_H
#define BOOTLOADER_KEY_H

#include <stdint.h>

struct Key {
    const uint8_t ed25519_key[32];
    const char name[16];
    const bool trusted;
};

#endif