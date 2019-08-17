#include "CryptoUtil.h"
#include <string.h>

unsigned int crypto::checksum(const unsigned int *hash) {
	unsigned int msg[16] = { 0 };
	unsigned int digest[8] = { 0 };

	// Insert network byte, shift everything right 1 byte
	msg[0] = 0x00; // main network
	msg[0] |= hash[0] >> 8u;
	msg[1] = (hash[0] << 24u) | (hash[1] >> 8u);
	msg[2] = (hash[1] << 24u) | (hash[2] >> 8u);
	msg[3] = (hash[2] << 24u) | (hash[3] >> 8u);
	msg[4] = (hash[3] << 24u) | (hash[4] >> 8u);
	msg[5] = (hash[4] << 24u) | 0x00800000u;

	// Padding and length
	msg[15] = 168;

	// Hash address
	sha256Init(digest);
	sha256(msg, digest);

	// Prepare to make a hash of the digest
	memset(msg, 0, 16 * sizeof(unsigned int));
	for (int i = 0; i < 8; i++) {
		msg[i] = digest[i];
	}

	msg[8] = 0x80000000;
	msg[15] = 256;

	sha256Init(digest);
	sha256(msg, digest);

	return digest[0];
}
