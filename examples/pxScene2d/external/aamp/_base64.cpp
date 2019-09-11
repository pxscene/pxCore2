/*
 * If not stated otherwise in this file or this component's license file the
 * following copyright and licenses apply:
 *
 * Copyright 2018 RDK Management
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
*/

/**
 * @file _base64.cpp
 * @brief optimized pair of base64 encode/decode implementations
 */


#include "_base64.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>


/**
 * @brief convert blob of binary data to ascii base64-encoded equivalent
 * @param src pointer to first byte of binary data to be encoded
 * @param len number of bytes to encode
 * @retval pointer to malloc'd cstring containing base64 encoded version of string
 * @retval NULL if insufficient memory to allocate base64-encoded copy
 * @note caller responsible for freeing returned cstring
 */
char *base64_Encode(const unsigned char *src, size_t len)
{
	static const char *mBase64IndexToChar =
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		"abcdefghijklmnopqrstuvwxyz"
		"0123456789"
		"+/"; // note: some older implementations use "-" instead of "/"
	size_t outLen = ((len + 2) / 3) * 4;
	char *outData = (char *)malloc(1 + outLen);
	if( outData )
	{ // aaaaaa aabbbb bbbbcc cccccc
		char *dst = outData;
		outData[outLen] = 0x00;
		for (;;)
		{
			if (len == 0) break;
			int in0 = src[0] >> 2;
			int in1 = (src[0] & 0x3) << 4;
			dst[0] = mBase64IndexToChar[in0];
			dst[1] = mBase64IndexToChar[in1];
			len--;
			if (len == 0)
			{
				dst[2] = '=';
				dst[3] = '=';
				break;
			}
			in1 |= (src[1] >> 4);
			int in2 = (src[1] & 0xf) << 2;
			dst[1] = mBase64IndexToChar[in1];
			dst[2] = mBase64IndexToChar[in2];
			len--;
			if (len == 0)
			{
				dst[3] = '=';
				break;
			}
			in2 |= src[2] >> 6;
			int in3 = src[2] & 0x3f;
			dst[2] = mBase64IndexToChar[in2];
			dst[3] = mBase64IndexToChar[in3];
			len--;
			src += 3;
			dst += 4;
		}
	}
	return outData;
}


/**
 * @brief decode base64 encoded data to binary equivalent
 * @param src pointer to cstring containing base64-encoded data
 * @param len receives byte length of returned pointer, or zero upon failure
 * @retval pointer to malloc'd memory containing decoded binary data
 * @retval NULL if insufficient memory to allocate base64-decoded data
 * @note caller responsible for freeing returned data
 */

unsigned char *base64_Decode(const char *src, size_t *len, size_t srcLen)
{
	static const signed char mBase64CharToIndex[256] =
	{
		/*
		Generated using:


		static int CodeToIndex( char c )
		{
		if (c >= 'A' && c <= 'Z') return c - 'A';
		if (c >= 'a' && c <= 'z') return (c - 'a') + 26;
		if (c >= '0' && c <= '9') return (c - '0') + 26 * 2;
		if (c == '+') return 62;
		if (c == '-') return 63;
		if (c == '/') return 63; // new
		return -1; // error
		}
		*/
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 62, -1, 63, -1, 63,
		52, 53, 54, 55, 56, 57, 58, 59, 60, 61, -1, -1, -1, -1, -1, -1,
		-1,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
		15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, -1, -1, -1, -1, -1,
		-1, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
		41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	};
	size_t numChars = (srcLen / 4) * 3; // initially round up to nearest 4 bytes
	unsigned char *outData = (unsigned char *)malloc(numChars); // 5538
	if (outData)
	{
		// memset(outData, 0x00, numChars); // not-needed
		unsigned char *dst = outData;
		const char *finish = src + srcLen;
		while (src < finish)
		{ // aaaaaa aabbbb bbbbcc cccccc
			int data0 = mBase64CharToIndex[(unsigned char)src[0]];
			int data1 = mBase64CharToIndex[(unsigned char)src[1]];
			int data2 = mBase64CharToIndex[(unsigned char)src[2]];
			int data3 = mBase64CharToIndex[(unsigned char)src[3]];
			*dst++ = (data0 << 2) | (data1 >> 4);
			if (data2 < 0)
			{
				numChars -= 2;
				break;
			}
			*dst++ = (data1 << 4) | (data2 >> 2);
			if (data3 < 0)
			{
				numChars--;
				break;
			}
			*dst++ = (data2 << 6) | (data3);
			src += 4;
		}
		*len = numChars; // 5536
	}
	else
	{ // insufficient memory
		*len = 0;
	}
	return outData;
}

unsigned char *base64_Decode(const char *src, size_t *len)
{
	return base64_Decode(src, len, strlen(src));
}
/**
 * @}
 */
