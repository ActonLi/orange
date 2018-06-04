#ifndef _ORANGE_ENDIAN_H_
#define _ORANGE_ENDIAN_H_

static inline uint16_t orange_be16dec(const void* pp)
{
	uint8_t const* p = (uint8_t const*) pp;

	return ((p[0] << 8) | p[1]);
}

static inline uint32_t orange_be32dec(const void* pp)
{
	uint8_t const* p = (uint8_t const*) pp;

	return (((unsigned) p[0] << 24) | (p[1] << 16) | (p[2] << 8) | p[3]);
}

static inline uint64_t orange_be64dec(const void* pp)
{
	uint8_t const* p = (uint8_t const*) pp;

	return (((uint64_t) orange_be32dec(p) << 32) | orange_be32dec(p + 4));
}

static inline uint16_t orange_le16dec(const void* pp)
{
	uint8_t const* p = (uint8_t const*) pp;

	return ((p[1] << 8) | p[0]);
}

static inline uint32_t orange_le32dec(const void* pp)
{
	uint8_t const* p = (uint8_t const*) pp;

	return (((unsigned) p[3] << 24) | (p[2] << 16) | (p[1] << 8) | p[0]);
}

static inline uint64_t orange_le64dec(const void* pp)
{
	uint8_t const* p = (uint8_t const*) pp;

	return (((uint64_t) orange_le32dec(p + 4) << 32) | orange_le32dec(p));
}

static inline void orange_be16enc(void* pp, uint16_t u)
{
	uint8_t* p = (uint8_t*) pp;

	p[0] = (u >> 8) & 0xff;
	p[1] = u & 0xff;
}

static inline void orange_be32enc(void* pp, uint32_t u)
{
	uint8_t* p = (uint8_t*) pp;

	p[0] = (u >> 24) & 0xff;
	p[1] = (u >> 16) & 0xff;
	p[2] = (u >> 8) & 0xff;
	p[3] = u & 0xff;
}

static inline void orange_be64enc(void* pp, uint64_t u)
{
	uint8_t* p = (uint8_t*) pp;

	orange_be32enc(p, (uint32_t)(u >> 32));
	orange_be32enc(p + 4, (uint32_t)(u & 0xffffffffU));
}

static inline void le16enc(void* pp, uint16_t u)
{
	uint8_t* p = (uint8_t*) pp;

	p[0] = u & 0xff;
	p[1] = (u >> 8) & 0xff;
}

static inline void orange_le32enc(void* pp, uint32_t u)
{
	uint8_t* p = (uint8_t*) pp;

	p[0] = u & 0xff;
	p[1] = (u >> 8) & 0xff;
	p[2] = (u >> 16) & 0xff;
	p[3] = (u >> 24) & 0xff;
}

static inline void orange_le64enc(void* pp, uint64_t u)
{
	uint8_t* p = (uint8_t*) pp;

	orange_le32enc(p, (uint32_t)(u & 0xffffffffU));
	orange_le32enc(p + 4, (uint32_t)(u >> 32));
}

#endif
