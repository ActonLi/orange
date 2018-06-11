/*
 * This code originally came from here
 *
 * http://base64.sourceforge.net/b64.c
 *
 * with the following license:
 *
 * LICENCE:        Copyright (c) 2001 Bob Trower, Trantor Standard Systems Inc.
 *
 *                Permission is hereby granted, free of charge, to any person
 *                obtaining a copy of this software and associated
 *                documentation files (the "Software"), to deal in the
 *                Software without restriction, including without limitation
 *                the rights to use, copy, modify, merge, publish, distribute,
 *                sublicense, and/or sell copies of the Software, and to
 *                permit persons to whom the Software is furnished to do so,
 *                subject to the following conditions:
 *
 *                The above copyright notice and this permission notice shall
 *                be included in all copies or substantial portions of the
 *                Software.
 *
 *                THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY
 *                KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 *                WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
 *                PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS
 *                OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 *                OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 *                OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 *                SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * VERSION HISTORY:
 *               Bob Trower 08/04/01 -- Create Version 0.00.00B
 *
 * I cleaned it up quite a bit to match the (linux kernel) style of the rest
 * of libwebsockets; this version is under LGPL2 like the rest of libwebsockets
 * since he explictly allows sublicensing, but I give the URL above so you can
 * get the original with Bob's super-liberal terms directly if you prefer.
 */


#include <stdio.h>
#include <string.h>
//#include "private-libwebsockets.h"

static const char encode[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
			     "abcdefghijklmnopqrstuvwxyz0123456789+/";
static const char decode[] = "|$$$}rstuvwxyz{$$$$$$$>?@ABCDEFGHIJKLMNOPQRSTUVW"
			     "$$$$$$XYZ[\\]^_`abcdefghijklmnopq";

int LWSB64EncodeString(const char *in, int in_len, char *out, int out_size)
{
	unsigned char triple[3];
	int i;
	int len;
	int line = 0;
	int done = 0;

	while (in_len) {
		len = 0;
		for (i = 0; i < 3; i++) {
			if (in_len) {
				triple[i] = *in++;
				len++;
				in_len--;
			} else
				triple[i] = 0;
		}
		if (!len)
			continue;

		if (done + 4 >= out_size)
		{
			printf("no space for encode\n");
			return -1;
		}

		*out++ = encode[triple[0] >> 2];
		*out++ = encode[((triple[0] & 0x03) << 4) |
					     ((triple[1] & 0xf0) >> 4)];
		*out++ = (len > 1 ? encode[((triple[1] & 0x0f) << 2) |
					     ((triple[2] & 0xc0) >> 6)] : '=');
		*out++ = (len > 2 ? encode[triple[2] & 0x3f] : '=');
		
		done += 4;
		line += 4;
	}
	if (done + 1 > out_size)
	{
		printf("no space for end mark\n");
		return -1;
	}
	

	*out++ = '\0';

	return done;
}

/*
 * returns length of decoded string in out, or -1 if out was too small
 * according to out_size
 */

int lws_b64_decode_string(const char *in, char *out, int out_size)
{
	int len;
	int i;
	int done = 0;
	unsigned char v;
	unsigned char quad[4];

	while (*in) {

		len = 0;
		for (i = 0; i < 4 && *in; i++) {

			v = 0;
			while (*in && !v) {

				v = *in++;
				v = (v < 43 || v > 122) ? 0 : decode[v - 43];
				if (v)
					v = (v == '$') ? 0 : v - 61;
				if (*in) {
					len++;
					if (v)
						quad[i] = v - 1;
				} else
					quad[i] = 0;
			}
		}
		if (!len)
			continue;

		if (out_size < (done + len - 1))
			/* out buffer is too small */
			return -1;

		if (len >= 2)
			*out++ = quad[0] << 2 | quad[1] >> 4;
		if (len >= 3)
			*out++ = quad[1] << 4 | quad[2] >> 2;
		if (len >= 4)
			*out++ = ((quad[2] << 6) & 0xc0) | quad[3];

		done += len - 1;
	}

	if (done + 1 >= out_size)
		return -1;

	*out++ = '\0';

	return done;
}

int lws_b64_selftest(void)
{
	char buf[64];
	int n;
	int test;
	static const char * const plaintext[] = {
		"sanity check base 64"
	};
	static const char * const coded[] = {
		"c2FuaXR5IGNoZWNrIGJhc2UgNjQ="
	};

	for (test = 0; test < sizeof plaintext / sizeof(plaintext[0]); test++) {

		buf[sizeof(buf) - 1] = '\0';
		n = LWSB64EncodeString(plaintext[test],
				      strlen(plaintext[test]), buf, sizeof buf);
		if (n != strlen(coded[test]) || strcmp(buf, coded[test])) {
			printf("Failed lws_b64 encode selftest "
					   "%d result '%s' %d\n", test, buf, n);
			return -1;
		}

		buf[sizeof(buf) - 1] = '\0';
		n = lws_b64_decode_string(coded[test], buf, sizeof buf);
		if (n != strlen(plaintext[test]) ||
						 strcmp(buf, plaintext[test])) {
			printf("Failed lws_b64 decode selftest "
					   "%d result '%s' %d\n", test, buf, n);
			return -1;
		}
	}

	return 0;
}



int Base64Enc(unsigned char *buf,const unsigned char*text,int size)
{
   static char *base64_encoding =
       "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
   int buflen = 0;

   while(size>0){
       *buf++ = base64_encoding[ (text[0] >> 2 ) & 0x3f];
       if(size>2){
           *buf++ = base64_encoding[((text[0] & 3) << 4) | (text[1] >> 4)];
           *buf++ = base64_encoding[((text[1] & 0xF) << 2) | (text[2] >> 6)];
           *buf++ = base64_encoding[text[2] & 0x3F];
       }else{
           switch(size){
               case 1:
                   *buf++ = base64_encoding[(text[0] & 3) << 4 ];
                   *buf++ = '=';
                   *buf++ = '=';
                   break;
               case 2:
                   *buf++ = base64_encoding[((text[0] & 3) << 4) | (text[1] >> 4)];
                   *buf++ = base64_encoding[((text[1] & 0x0F) << 2) | (text[2] >> 6)];
                   *buf++ = '=';
                   break;
           }
       }

       text +=3;
       size -=3;
       buflen +=4;
   }

   *buf = 0;
   return buflen;
}

char GetBase64Value(char ch)
{
   if ((ch >= 'A') && (ch <= 'Z'))
       return ch - 'A';
   if ((ch >= 'a') && (ch <= 'z'))
       return ch - 'a' + 26;
   if ((ch >= '0') && (ch <= '9'))
       return ch - '0' + 52;
   switch (ch) {
       case '+':
           return 62;
       case '/':
           return 63;
       case '=':
           return 0;
       default:
           return 0;
   }
}


int Base64Dec(unsigned char * buf, char * text, int size)
{
   if(size%4)
       return -1;
   unsigned char chunk[4];
   int parsenum=0;

   while(size>0){
       chunk[0] = GetBase64Value(text[0]);
       chunk[1] = GetBase64Value(text[1]);
       chunk[2] = GetBase64Value(text[2]);
       chunk[3] = GetBase64Value(text[3]);

       *buf++ = (chunk[0] << 2) | (chunk[1] >> 4);
       *buf++ = (chunk[1] << 4) | (chunk[2] >> 2);
       *buf++ = (chunk[2] << 6) | (chunk[3]);

       text+=4;
       size-=4;
       parsenum+=3;
   }

   return parsenum;
}

