#include <ion.h>
#include "../external/ed25519/ed25519.h"

bool Ion::verify(const uint8_t signature[64], const uint8_t *message, size_t message_len, const uint8_t public_key[32]) {
  return ed25519_verify(signature, message, message_len, public_key);
}