#ifndef __ORANGE_UTILS_H__
#define __ORANGE_UTILS_H__

#include "orange.h"

extern uint32_t orange_atoi(const char* s);
extern uint64_t orange_atoi_64(const char* s);
extern int orange_itoa(int num, char* s, int* len);
extern void orange_str_reverse(char* str);

extern char* orange_url_encode(char const* s, int len, int* new_length);
extern int orange_url_decode(char* str, int len);

extern int orange_strlen(unsigned char* start_string, unsigned char* stop_string);
extern unsigned char* orange_strstr(unsigned char* start_string, unsigned char* stop_string, unsigned char* str);

extern unsigned char* orange_strchr_reverse(char* start_string, char* stop_string, char ch);
extern unsigned char* orange_strchr(unsigned char* start_string, unsigned char* stop_string, char ch);

extern unsigned char* orange_line_end(unsigned char* start_string, unsigned char* stop_string);
extern int orange_strnicmp(const char* s1, const char* s2, size_t count);

extern int orange_proc_name(char* proc_name, int size);

extern char* orange_get_short_proc_name(char* p_name);

static inline int orange_before(uint32_t seq1, uint32_t seq2)
{
	return (seq1 < seq2) ? 1 : 0;
}

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

static inline uint64_t orange_htonll(uint64_t n)
{
	uint64_t retval;
	retval = ((uint64_t) htonl(n & 0xFFFFFFFFLLU)) << 32;
	retval |= htonl((n & 0xFFFFFFFF00000000LLU) >> 32);

	return retval;
}

static inline uint64_t orange_ntohll(uint64_t n)
{
	uint64_t retval;
	retval = ((uint64_t) htonl(n & 0xFFFFFFFFLLU)) << 32;
	retval |= htonl((n & 0xFFFFFFFF00000000LLU) >> 32);

	return retval;
}

static inline int __orange_get_hex_char(char in)
{
	if (in >= '0' && in <= '9') {
		return in - '0';
	} else {
		if (in >= 'a' && in <= 'f') {
			return (in - 'a' + 10);
		} else {
			if (in >= 'A' && in <= 'F') {
				return (in - 'A' + 10);
			}
		}
	}

	return 0;
}

/* converting the hex char to int, which is used to covert chunk data length */
static inline int orange_convert_hex_char(char* input, int len)
{
	int ret = 0;
	int i;

	if (len >= 1 && len <= 8) {
		for (i = 0; i < len; i++) {
			int r = __orange_get_hex_char(input[i]);
			ret <<= 4;
			ret += r;
		}
	}

	return ret;
}

static inline void orange_ltrim(char* s)
{
	char* p;

	p = s;
	while (*p == ' ' || *p == '\t') {
		p++;
	}

	strcpy(s, p);
}

static inline void orange_rtrim(char* s)
{
	int i;

	i = strlen(s) - 1;

	while ((s[i] == ' ' || s[i] == '\t') && i >= 0) {
		i--;
	}

	s[i + 1] = '\0';
}

static inline void orange_trim(char* s)
{
	orange_ltrim(s);
	orange_rtrim(s);
}

uint64_t orange_hexstr_to_hex64(char* s, int* is_more, int* bits);

#endif //__ORANGE_UTILS_H__
