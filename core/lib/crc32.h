/*
 * The original license details for the work by Grant P. Maizels and
 * Ross Williams, code amalgamation by James Bensley Copyright (c) 2018.
 *
 ******************************************************************************
 * Copyright (C) 2000 Grant P. Maizels. All rights reserved
 * This software is copyrighted work licensed under the terms of the
 * GNU General Public License. Please consult
 * http://www.gnu.org/licenses/licenses.html#GPL for details.
 * Version 1.0 last update 19Nov2001
 ******************************************************************************
 *
 * Author : Ross Williams (ross@guest.adelaide.edu.au.).
 * Date   : 3 June 1993.
 * Status : Public domain.
 *
 * Description : This is the header (.h) file for the reference
 * implementation of the Rocksoft^tm Model CRC Algorithm. For more
 * information on the Rocksoft^tm Model CRC Algorithm, see the document
 * titled "A Painless Guide to CRC Error Detection Algorithms" by Ross
 * Williams (ross@guest.adelaide.edu.au.). This document is likely to be in
 * "ftp.adelaide.edu.au/pub/rocksoft".
 *
 * Note: Rocksoft is a trademark of Rocksoft Pty Ltd, Adelaide, Australia.
 *
 ******************************************************************************
 */


#ifndef _CRC32_H_
#define _CRC32_H_


#define MAX_FRAME_SIZE 10000
#define BITMASK(X) (1L << (X))


/* CRC Model Abstract Type */
/* ----------------------- */
/* The following type stores the context of an executing instance of the  */
/* model algorithm. Most of the fields are model parameters which must be */
/* set before the first initializing call to cm_ini.                      */
typedef struct {
    uint32_t cm_width;   /* Parameter: Width in bits [8,32].       */
    uint32_t cm_poly;    /* Parameter: The algorithm's polynomial. */
    uint32_t cm_init;    /* Parameter: Initial register value.     */
    uint8_t  cm_refin;   /* Parameter: Reflect input bytes?        */
    uint8_t  cm_refot;   /* Parameter: Reflect output CRC?         */
    uint32_t cm_xorot;   /* Parameter: XOR this to output CRC.     */
    uint32_t cm_reg;     /* Context: Context during execution.     */
} cm_t;

typedef cm_t *p_cm_t;



uint32_t crc32_data(const unsigned char *data, int len, uint32_t crc_init);

#endif  // _CRC32_H_
