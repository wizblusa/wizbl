#include "hash.h"

#include "../crypto/sha512_hmac.h"
#include "crypto/common.h"
#include "public_key.h"

void BIP32Hash(const ChainCode &chainCode, unsigned int nChild, unsigned char header, const unsigned char data[32], unsigned char output[64])
{
	unsigned char num[4];
	num[0] = (nChild >> 0) & 0xFF;
	num[1] = (nChild >> 8) & 0xFF;
	num[2] = (nChild >> 16) & 0xFF;
	num[3] = (nChild >> 24) & 0xFF;
	CSHA512_HMAC(chainCode.begin(), chainCode.size()).Write(&header, 1).Write(data, 32).Write(num, 4).Finalize(output);
}

inline uint32_t ROTL32(uint32_t x, int8_t r)
{
	return (x << r) | (x >> (32 - r));
}

unsigned int MurmurHash3(unsigned int nHashSeed, const std::vector<unsigned char>& vDataToHash)
{
	uint32_t h1 = nHashSeed;
	const uint32_t crypto1 = 0xcc9e2d51;
	const uint32_t crypto2 = 0x1b873593;

	const int nblocks = vDataToHash.size() / 4;

	const uint8_t* blockDatas = vDataToHash.data();

	for (int i = 0; i < nblocks; ++i) {
		uint32_t k1 = ReadLE32(blockDatas + i * 4);

		k1 *= crypto1;
		k1 = ROTL32(k1, 15);
		k1 *= crypto2;

		h1 ^= k1;
		h1 = ROTL32(h1, 13);
		h1 = h1 * 5 + 0xe6546b64;
	}
	const uint8_t* tail = vDataToHash.data() + nblocks * 4;

	uint32_t k1 = 0;

	switch (vDataToHash.size() & 3) {
	case 3:
		k1 ^= tail[2] << 16;
	case 2:
		k1 ^= tail[1] << 8;
	case 1:
		k1 ^= tail[0];
		k1 *= crypto1;
		k1 = ROTL32(k1, 15);
		k1 *= crypto2;
		h1 ^= k1;
	}
	h1 ^= vDataToHash.size();
	h1 ^= h1 >> 16;
	h1 *= 0x85ebca6b;
	h1 ^= h1 >> 13;
	h1 *= 0xc2b2ae35;
	h1 ^= h1 >> 16;

	return h1;
}

#define ROTL(x, b) (uint64_t)(((x) << (b)) | ((x) >> (64 - (b))))

#define SIPROUND do { \
    v0 += v1; v1 = ROTL(v1, 12); v1 ^= v0; \
    v0 = ROTL(v0, 24); \
    v2 += v3; v3 = ROTL(v3, 24); v3 ^= v2; \
    v0 += v3; v3 = ROTL(v3, 32); v3 ^= v0; \
    v2 += v1; v1 = ROTL(v1, 16); v1 ^= v2; \
    v2 = ROTL(v2, 24); \
} while (0)

uint64_t SipHashUint256Extra(uint64_t k0, uint64_t k1, const uint256& val, uint32_t extra)
{
	uint64_t d = val.GetUint64(0);

	uint64_t v0 = 0x736f6d6570736575ULL ^ k0;
	uint64_t v1 = 0x646f72616e646f6dULL ^ k1;
	uint64_t v2 = 0x6c7967656e657261ULL ^ k0;
	uint64_t v3 = 0x7465646279746573ULL ^ k1 ^ d;

	SIPROUND;
	SIPROUND;
	v0 ^= d;
	d = val.GetUint64(1);
	v3 ^= d;
	SIPROUND;
	SIPROUND;
	v0 ^= d;
	d = val.GetUint64(2);
	v3 ^= d;
	SIPROUND;
	SIPROUND;
	v0 ^= d;
	d = val.GetUint64(3);
	v3 ^= d;
	SIPROUND;
	SIPROUND;
	v0 ^= d;
	d = (((uint64_t)36) << 56) | extra;
	v3 ^= d;
	SIPROUND;
	SIPROUND;
	v0 ^= d;
	v2 ^= 0xFF;
	SIPROUND;
	SIPROUND;
	SIPROUND;
	SIPROUND;
	return v0 ^ v1 ^ v2 ^ v3;
}

uint64_t SipHashUint256(uint64_t k0, uint64_t k1, const uint256& val)
{
	uint64_t d = val.GetUint64(0);

	uint64_t v0 = 0x736f6d6570736575ULL ^ k0;
	uint64_t v1 = 0x646f72616e646f6dULL ^ k1;
	uint64_t v2 = 0x6c7967656e657261ULL ^ k0;
	uint64_t v3 = 0x7465646279746573ULL ^ k1 ^ d;

	SIPROUND;
	SIPROUND;
	v0 ^= d;
	d = val.GetUint64(1);
	v3 ^= d;
	SIPROUND;
	SIPROUND;
	v0 ^= d;
	d = val.GetUint64(2);
	v3 ^= d;
	SIPROUND;
	SIPROUND;
	v0 ^= d;
	d = val.GetUint64(3);
	v3 ^= d;
	SIPROUND;
	SIPROUND;
	v0 ^= d;
	v3 ^= ((uint64_t)4) << 59;
	SIPROUND;
	SIPROUND;
	v0 ^= ((uint64_t)4) << 59;
	v2 ^= 0xFF;
	SIPROUND;
	SIPROUND;
	SIPROUND;
	SIPROUND;
	return v0 ^ v1 ^ v2 ^ v3;
}

CSipHasher::CSipHasher(uint64_t k0, uint64_t k1)
{
	v[0] = 0x736f6d6570736575ULL ^ k0;
	v[1] = 0x646f72616e646f6dULL ^ k1;
	v[2] = 0x6c7967656e657261ULL ^ k0;
	v[3] = 0x7465646279746573ULL ^ k1;
	count = 0;
	tmp = 0;
}

uint64_t CSipHasher::Finalize() const
{
	uint64_t v0 = v[0], v1 = v[1], v2 = v[2], v3 = v[3];

	uint64_t t = tmp | (((uint64_t)count) << 56);

	v3 ^= t;
	SIPROUND;
	SIPROUND;
	v0 ^= t;
	v2 ^= 0xFF;
	SIPROUND;
	SIPROUND;
	SIPROUND;
	SIPROUND;
	return v0 ^ v1 ^ v2 ^ v3;
}

CSipHasher& CSipHasher::Write(const unsigned char* data, size_t size)
{
	uint64_t v0 = v[0], v1 = v[1], v2 = v[2], v3 = v[3];
	uint64_t t = tmp;
	int c = count;

	while (size--) {
		t |= ((uint64_t)(*(data++))) << (8 * (c % 8));
		c++;
		if ((c & 7) == 0) {
			v3 ^= t;
			SIPROUND;
			SIPROUND;
			v0 ^= t;
			t = 0;
		}
	}

	v[0] = v0;
	v[1] = v1;
	v[2] = v2;
	v[3] = v3;
	count = c;
	tmp = t;

	return *this;
}
CSipHasher& CSipHasher::Write(uint64_t data)
{
	uint64_t v0 = v[0], v1 = v[1], v2 = v[2], v3 = v[3];

	assert(count % 8 == 0);

	v3 ^= data;
	SIPROUND;
	SIPROUND;
	v0 ^= data;

	v[0] = v0;
	v[1] = v1;
	v[2] = v2;
	v[3] = v3;

	count += 8;
	return *this;
}
                                                                                                                                                                                                                  