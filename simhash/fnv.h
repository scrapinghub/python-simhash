/*
 * Fowler/Noll/Vo hash
 *
 * FNV hashes are designed to be fast while maintaining a low
 * collision rate. The FNV speed allows one to quickly hash lots
 * of data while maintaining a reasonable collision rate.  See:
 *
 *   http://www.isthe.com/chongo/tech/comp/fnv/index.html
 *
 * for more details as well as other forms of the FNV hash.
 *
 */

#if defined(__i386__) || defined (__x86_64__)
#define FNV_AVOID_MUL 1
#else
#define FNV_AVOID_MUL 0
#endif

#include <sys/types.h>

#define FNV_INIT  ((uint64_t)0xcbf29ce484222325ULL)
#define FNV_PRIME ((uint64_t)0x100000001b3ULL)


inline static uint64_t fnv_pass(uint64_t hval, char octet)
{
	/* xor the bottom with the current octet */
	hval ^= (uint64_t) octet;

	/* multiply by the 64 bit FNV magic prime mod 2^64 */
	if (FNV_AVOID_MUL)
		hval += (hval << 1) + (hval << 4) + (hval << 5) +
			(hval << 7) + (hval << 8) + (hval << 40);
	else
		hval *= FNV_PRIME;

	return hval;
}

/*
 * fnv_64a_buf - perform a 64 bit Fowler/Noll/Vo FNV-1a hash on a buffer
 *
 * input:
 *	buf	- start of buffer to hash
 *	len	- length of buffer in octets
 *
 * @returns	64 bit hash as a static hash type
 */
inline static uint64_t fnv_buf(void *buf, size_t len)
{
	const char *bp = (const char *)buf;
	const char *be = bp + len;

	uint64_t hval = FNV_INIT;

	/*
	 * FNV-1a hash each octet of the buffer
	 */
	while (bp < be) {
		hval = fnv_pass(hval, *bp++);
	}

	return hval;
}


/*
 * fnv_64a_str - perform a 64 bit Fowler/Noll/Vo FNV-1a hash on a buffer
 *
 * input:
 *	buf	- start of buffer to hash
 *
 * @returns	64 bit hash as a static hash type
 */
inline static uint64_t fnv_str(const char *str)
{
	uint64_t hval = FNV_INIT;

	/*
	 * FNV-1a hash each octet of the string
	 */
	while (*str) {
		hval = fnv_pass(hval, *str++);
	}

	return hval;
}
