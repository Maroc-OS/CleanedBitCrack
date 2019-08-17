#include "CryptoUtil.h"
#include <stdexcept>
#include <string>

#ifdef _WIN32
#include <Windows.h>
#include <bcrypt.h>

static void secureRandom(unsigned char* buf, unsigned int count)
{
    BCRYPT_ALG_HANDLE h;
    BCryptOpenAlgorithmProvider(&h, BCRYPT_RNG_ALGORITHM, NULL, 0);
    BCryptGenRandom(h, buf, count, 0);
}
#else

static void secureRandom(unsigned char *buf, size_t count) {
    // Read from /dev/urandom
    FILE* urandom = fopen("/dev/urandom", "rb");
    size_t bytes_read = 0;

    if (!urandom) {
        throw std::runtime_error("Fatal error: Cannot open urandom device for reading");
    }

    bytes_read = fread(buf, 1, count, urandom);

    if (bytes_read != count) {
        fclose(urandom);
        throw std::runtime_error("Fatal error: Not enough entropy available in urandom device");
    }

    fclose(urandom);
}

#endif

crypto::Rng::Rng() {
	reseed();
}

void crypto::Rng::reseed() {
	_counter = 0;

	memset(_state, 0, sizeof(_state));

	secureRandom((unsigned char *) _state, 32);
}

void crypto::Rng::get(unsigned char *buf, size_t len) {
	int i = 0;
	size_t new_len = len;
	while (len > 0) {
		if (_counter++ == 0xffffffff) {
			reseed();
		}

		_state[15] = _counter;

		unsigned int digest[8];
		sha256Init(digest);
		sha256(_state, digest);

		if (len >= 32) {
			memcpy(&buf[i], (const void *) digest, 32);
			i += 32;
			len -= 32;
		} else {
			memcpy(&buf[i], (const void *) digest, new_len);
			i += new_len;
			len -= new_len;
		}
	}
}
