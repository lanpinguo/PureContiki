/**
 * \file
 *         Implementation of the CRC32 calculcation
 * \author
 *         Lanpinguo <lanpinguo1985@126.com>
 *
 */
#include <stdio.h>     // EOF, FILE, fclose(), fprintf(), fscanf(), perror(), printf(), stderr, stdin
#include <stdlib.h>    // calloc(), exit()
#include <string.h>    // memset()

#include "crc32.h"




/*---------------------------------------------------------------------------*/
/* CITT CRC32 polynomial 
x^32 + x^26 + x^23 + x^22 + x^16 + x^12 + x^11+ x^10 + x^8 + x^7 + x^5 + x^4 + x^2 + x + 1 
HEX - 0x04C11DB7
*/
unsigned short
crc32_add(unsigned char b, unsigned int acc)
{
	acc ^= b;
	acc  = (acc >> 8) | (acc << 8);
	acc ^= (acc & 0xff00) << 4;
	acc ^= (acc >> 8) >> 4;
	acc ^= (acc & 0xff00) >> 5;
	return acc;
}
/*---------------------------------------------------------------------------*/
unsigned int
crc32_data(const unsigned char *data, int len, unsigned int acc)
{
	int i;

	for(i = 0; i < len; ++i) {
	acc = crc32_add(*data, acc);
		++data;
	}
	return acc;
}
/*---------------------------------------------------------------------------*/


