#include "orange_utils.h"
#include <errno.h>

static unsigned char hexchars[] = "0123456789ABCDEF";

static int __htoi(char* s)
{
	int value;
	int c;

	c = ((unsigned char*) s)[0];
	if (isupper(c))
		c = tolower(c);
	value = (c >= '0' && c <= '9' ? c - '0' : c - 'a' + 10) * 16;

	c = ((unsigned char*) s)[1];
	if (isupper(c))
		c = tolower(c);
	value += c >= '0' && c <= '9' ? c - '0' : c - 'a' + 10;

	return (value);
}

char* orange_url_encode(char const* s, int len, int* new_length)
{
	register unsigned char c;
	unsigned char *		   to, *start;
	unsigned char const *  from, *end;

	from  = (unsigned char const*) s;
	end   = (unsigned char const*) s + len;
	start = to = (unsigned char*) orange_zalloc(3 * len + 1);

	while (from < end) {
		c = *from++;

		if (c == ' ') {
			*to++ = '+';
		} else if ((c < '0' && c != '-' && c != '.') || (c < 'A' && c > '9') || (c > 'Z' && c < 'a' && c != '_') || (c > 'z')) {
			to[0] = '%';
			to[1] = hexchars[c >> 4];
			to[2] = hexchars[c & 15];
			to += 3;
		} else {
			*to++ = c;
		}
	}
	*to = 0;
	if (new_length) {
		*new_length = to - start;
	}
	return (char*) start;
}

int orange_url_decode(char* str, int len)
{
	char* dest = str;
	char* data = str;

	while (len--) {
		if (*data == '+') {
			*dest = ' ';
		} else if (*data == '%' && len >= 2 && isxdigit((int) *(data + 1)) && isxdigit((int) *(data + 2))) {
			*dest = (char) __htoi(data + 1);
			data += 2;
			len -= 2;
		} else {
			*dest = *data;
		}
		data++;
		dest++;
	}
	*dest = '\0';
	return dest - str;
}

uint32_t orange_atoi(const char* s)
{
	uint32_t val = 0;

	for (;; s++) {
		switch (*s) {
			case '0' ... '9':
				val = 10 * val + (*s - '0');
				break;
			default:
				return val;
		}
	}
}

uint64_t orange_atoi_64(const char* s)
{
	uint64_t val = 0;

	for (;; s++) {
		switch (*s) {
			case '0' ... '9':
				val = 10 * val + (*s - '0');
				break;
			default:
				return val;
		}
	}
}

void orange_str_reverse(char* str)
{
	char* end = str;
	char  temp;

	if (str) {
		while (*end) {
			++end;
		}

		--end;

		while (str < end) {
			temp   = *str;
			*str++ = *end;
			*end-- = temp;
		}
	}
}

int orange_itoa(int num, char* s, int* len)
{
	int ret			= 0;
	int is_negative = 0;
	int curlen		= 0;

	if (num < 0) {
		is_negative = 1;
		num			= -num;
	}

	do {
		s[curlen++] = num % 10 + '0';
		num			= num / 10;
	} while (num > 0);

	if (is_negative) {
		s[curlen++] = '-';
	}

	s[curlen] = '\0';
	*len	  = curlen;

	orange_str_reverse(s);

	return ret;
}

extern int orange_strlen(unsigned char* start_string, unsigned char* stop_string)
{
	int len = 0;
	while (*start_string != '\0') {
		if (start_string >= (stop_string - 1)) {
			return len;
		}
		start_string++;
		len++;
	}
	return len;
}

extern unsigned char* orange_strstr(unsigned char* start_string, unsigned char* stop_string, unsigned char* str)
{
compare:
	while (*start_string != *str) {
		if (*start_string == 0 || start_string >= stop_string) {
			return NULL;
		}
		start_string++;
	}

	if (start_string + strlen((char*) str) > stop_string) {
		return NULL;
	}

	if (strncmp((char*) start_string, (char*) str, strlen((char*) str)) != 0) {
		start_string++;
		goto compare;
	}

	return start_string;
}

unsigned char* orange_strchr(unsigned char* start_string, unsigned char* stop_string, char ch)
{
	if ((start_string == NULL) || (stop_string == NULL) || (start_string > stop_string))
		return NULL;

	while (*start_string != ch) {
		if (start_string >= stop_string) {
			return NULL;
		}
		start_string++;
	}
	return start_string;
}

unsigned char* orange_strchr_reverse(char* start_string, char* stop_string, char ch)
{
	if ((start_string == NULL) || (stop_string == NULL) || (start_string > stop_string)) {
		return NULL;
	}

	while (*stop_string != ch) {
		if (stop_string <= start_string) {
			return NULL;
		}
		stop_string--;
	}

	return stop_string;
}

unsigned char* orange_line_end(unsigned char* start_string, unsigned char* stop_string)
{
	if ((start_string == NULL) || (stop_string == NULL) || (start_string >= stop_string))
		return NULL;

do_char_compare:

	while (*start_string != '\r') {
		if (start_string >= (stop_string - 1)) {
			return NULL;
		}
		start_string++;
	}

	start_string++;
	if (*start_string == '\n') {
		start_string--;
		return start_string;
	}
	goto do_char_compare;

	return NULL;
}

/* Case insensitive strncmp. Non-ISO, deprecated. */
int orange_strnicmp(const char* s1, const char* s2, size_t count)
{
	char c1, c2;
	int  v;

	if (count == 0)
		return 0;

	do {
		c1 = *s1++;
		c2 = *s2++;
		/* the casts are necessary when pStr1 is shorter & char is signed */
		v = (unsigned int) tolower(c1) - (unsigned int) tolower(c2);
	} while ((v == 0) && (c1 != '\0') && (--count > 0));

	return v;
}

char* orange_get_short_proc_name(char* p_name)
{
	char* p = p_name;

	if (p_name == NULL) {
		return NULL;
	}

	p = p_name + strlen(p_name) - 1;

	while (1) {
		if (p == p_name || *p == '/') {
			break;
		}
		p--;
	}

	if (*p == '/') {
		p++;
	}

	return p;
}

int orange_proc_name(char* proc_name, int size)
{
	int ret = EINVAL;

	// strncpy(proc_name, program_invocation_short_name, size);
	ret = 0;
	return ret;
}

/* TODO 奇数长度时，高位补零 */
uint64_t orange_hexstr_to_hex64(char* s, int* is_more, int* bits)
{
	uint64_t val = 0;
	int		 i   = 1;

	for (;; s++, i++) {
		switch (*s) {
			case '0' ... '9':
				if (i > (sizeof(uint64_t) << 1)) {
					*is_more = 1;
					break;
				}
				if ((i & 0x01)) {
					val = (val << 8);
					val |= (*s - '0') << 4;
				} else {
					val |= (*s - '0');
				}

				break;
			case 'a' ... 'z':
				if (i > (sizeof(uint64_t) << 1)) {
					*is_more = 1;
					break;
				}
				if ((i & 0x01)) {
					val = (val << 8);
					val |= (*s - 'a' + 10) << 4;
				} else {
					val |= (*s - 'a' + 10);
				}
				break;
			case 'A' ... 'Z':
				if (i > (sizeof(uint64_t) << 1)) {
					*is_more = 1;
					break;
				}
				if ((i & 0x01)) {
					val = (val << 8);
					val |= (*s - 'A' + 10) << 4;
				} else {
					val |= (*s - 'A' + 10);
				}
				break;
			default:
				if (bits != NULL) {
					*bits = ((i - 1) << 2);
				}
				return val;
		}

		if (*is_more == 1) {
			break;
		}
	}

	if (bits != NULL) {
		*bits = ((i - 1) << 2);
	}
	return val;
}
