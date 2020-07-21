
/* 
 * Based upon the work of Grant P. Maizels and Ross Williams.
 * Their original licence details are in the header file.
 * This amalgamation is the work of James Bensley Copyright (c) 2018.
 */ 


#include <stdio.h>     // EOF, FILE, fclose(), fprintf(), fscanf(), perror(), printf(), stderr, stdin
#include <stdlib.h>    // calloc(), exit()
#include <string.h>    // memset()

#include "crc32.h"

/* Functions That Implement The Model */
/* ---------------------------------- */
/* The following functions animate the cm_t abstraction. */

static void cm_ini (p_cm_t p_cm);
/* Initializes the argument CRC model instance.          */
/* All parameter fields must be set before calling this. */

static void cm_nxt (p_cm_t p_cm, uint32_t ch);
/* Processes a single message byte [0,255]. */

static void cm_blk (p_cm_t p_cm, uint8_t *blk_adr, uint32_t blk_len);
/* Processes a block of message bytes. */

static uint32_t cm_crc (p_cm_t p_cm);
/* Returns the CRC value for the message bytes processed so far. */


/* Functions For Table Calculation */
/* ------------------------------- */
/* The following function can be used to calculate a CRC lookup table.        */
/* It can also be used at run-time to create or check static tables.          */

static uint32_t cm_tab (p_cm_t p_cm, uint32_t index);
/* Returns the i'th entry for the lookup table for the specified algorithm.   */
/* The function examines the fields cm_width, cm_poly, cm_refin, and the      */
/* argument table index in the range [0,255] and returns the table entry in   */
/* the bottom cm_width bytes of the return value.                             */


static uint32_t reflect (uint32_t v, uint32_t b);
/* Returns the value v with the bottom b [0,32] bits reflected. */
/* Example: reflect(0x3e23L,3) == 0x3e26                        */

static uint32_t widmask (p_cm_t);
/* Returns a longword whose value is (2^p_cm->cm_width)-1.     */
/* The trick is to do this portably (e.g. without doing <<32). */

cm_t g_cm = {
  .cm_width  = 32,
  .cm_poly   = 0x04C11DB7,
  .cm_init   = 0xFFFFFFFF,
  .cm_refin  = 1,
  .cm_refot  = 1,
  .cm_xorot  = 0xFFFFFFFF,
};
p_cm_t p_cm = &g_cm;


uint32_t crc32_data(const unsigned char *data, int len, uint32_t crc_init)
{

  int max = len;
  int j;


  if (data == NULL) {
     return 0;
  }
    
  p_cm->cm_init   = crc_init;
  cm_ini(p_cm);
   

  for (j = 0; j < max; j++ ) {
    cm_nxt(p_cm, data[j]);
  }

  return cm_crc(p_cm);
}


static uint32_t reflect(uint32_t v, uint32_t b) {

  static int   i;
  static uint32_t t;

  t = v;

  for (i=0; i < b; i++) {

    if (t & 1L) {
       v|=  BITMASK((b - 1) - i);
    } else {
       v&= ~BITMASK((b - 1) - i);
    }

    t>>=1;

  }

  return v;

}


static uint32_t widmask(p_cm_t p_cm) {

  return (((1L << (p_cm->cm_width - 1)) - 1L) << 1) | 1L;

}


static void cm_ini(p_cm_t p_cm) {

  p_cm->cm_reg = p_cm->cm_init;

}


static void cm_nxt(p_cm_t p_cm, uint32_t ch) {

  static int   i;
  static uint32_t uch, topbit;

  uch    = ch;
  topbit = BITMASK(p_cm->cm_width - 1);

  if (p_cm->cm_refin) uch = reflect(uch, 8);

  p_cm->cm_reg ^= (uch << (p_cm->cm_width - 8));

  for (i=0; i < 8; i++) {

    if (p_cm->cm_reg & topbit) {
      p_cm->cm_reg = (p_cm->cm_reg << 1) ^ p_cm->cm_poly;
    } else {
       p_cm->cm_reg <<= 1;
    }

    p_cm->cm_reg &= widmask(p_cm);

  }

}


static void cm_blk(p_cm_t p_cm, uint8_t *blk_adr, uint32_t blk_len) {

  while (blk_len--) cm_nxt(p_cm, *blk_adr++);

}


static uint32_t cm_crc(p_cm_t p_cm) {

  if (p_cm->cm_refot) {
    return p_cm->cm_xorot ^ reflect(p_cm->cm_reg, p_cm->cm_width);
  } else {
    return p_cm->cm_xorot ^ p_cm->cm_reg;
  }

}


static uint32_t cm_tab(p_cm_t p_cm, uint32_t index) {

  static uint8_t  i;
  static uint32_t r, topbit, inbyte;

  topbit = BITMASK(p_cm->cm_width - 1);
  inbyte = index;

  if (p_cm->cm_refin) inbyte = reflect(inbyte, 8);

  r = inbyte << (p_cm->cm_width - 8);

  for (i=0; i < 8; i++) {

    if (r & topbit) {
      r = (r << 1) ^ p_cm->cm_poly;
    } else {
       r <<= 1;
     }
  }

  if (p_cm->cm_refin) r = reflect(r, p_cm->cm_width);

  return r & widmask(p_cm);

}

