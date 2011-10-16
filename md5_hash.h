#ifndef MD5_HASH_H
#define MD5_HASH_H

#include <stdlib.h>
#include <stdint.h>
#include <math.h>

#define BITS_PER_BYTE   8
#define BYTES_TO_BITS(b) (b * BITS_PER_BYTE)

/* ==================================== */
/* Size values as specified by RFC 1321 */
/* ==================================== */

#define MD5_CHUNK_BITS      512
#define MD5_RESULT_BITS     128

#define MD5_CHUNK_LENGTH    64  /* 64 bytes = 512 bits */
#define MD5_RESULT_LENGTH   16  /* 16 bytes = 128 bits */
#define NUM_WORDS_PER_CHUNK  16  /* 16 32-bit ints = 512 bits */
#define NUM_WORDS_RESULT     4   /* 4 32-bit ints = 128-bit result */

/* ==================================== */
/* Word indices for readability.        */
/* The RFC uses these names, and it'll  */
/* make the code more readable. :P      */
/* ==================================== */
#define A   0
#define B   1
#define C   2
#define D   3

#define a   0
#define b   1
#define c   2
#define d   3

/* ==================================== */
/* Left-circular shift (RFC 1321)       */
/* ==================================== */

/* ROTATE_LEFT rotates x left n bits */
#define ROTATE_LEFT(x, n) (((x) << (n)) | ((x) >> (32-(n))))



/* ==================================== */
/* Typedefs for clarity and readability */
/* ==================================== */

typedef uint32_t word;
typedef unsigned char byte; /* because byte makes more sense */
                            /* than unsigned char for denoting 8 bits. */

typedef word (*md5_round_function)(word, word, word);
typedef word md5_chunk[NUM_WORDS_PER_CHUNK]; /* 512-bit block buffer */
typedef word md5_result[NUM_WORDS_RESULT];   /* 128-bit result */


/* ==================================== */
/* Values for each round (See RFC 1321) */
/* ==================================== */

/* These values are used when performing the round operations.
 * According to the RFC the operation looks like this:
 *
 * Let [abcd k s i] denote the operation
 *           a = b + ((a + F(b,c,d) + X[k] + T[i]) <<< s).
 */

/* The abcd values for each round follows these */
/* values for every 4th operation */
/* (i.e. the first gets done on the 1st, 5th, 9th, and 13th  */
/* operation, and so on...) */
int word_orders[NUM_WORDS_RESULT][NUM_WORDS_RESULT] = 
{
    {A, B, C, D},   /* operation #0 mod 4 */
    {D, A, B, C},   /* operation #1 mod 4 */
    {C, D, A, B},   /* operation #2 mod 4 */
    {B, C, D, A}    /* operation #3 mod 4 */
};

/* the s values for each round. */
const int shift_amounts[NUM_WORDS_RESULT][NUM_WORDS_RESULT] =
{
    { 7, 12, 17, 22},    /* round 1 */
    { 5,  9, 14, 20},    /* round 2 */
    { 4, 11, 16, 23},    /* round 3 */
    { 6, 10, 15, 21},    /* round 4 */
};

/*  The k values for each round */
const int k_values[NUM_WORDS_RESULT][NUM_WORDS_PER_CHUNK] = 
{
    {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15}, /* round 1 */
    {1, 6, 11, 0, 5, 10, 15, 4, 9, 14, 3, 8, 13, 2, 7, 12}, /* round 2 */
    {5, 8, 11, 14, 1, 4, 7, 10, 13, 0, 3, 6, 9, 12, 15, 2}, /* round 3 */
    {0, 7, 14, 5, 12, 3, 10, 1, 8, 15, 6, 13, 4, 11, 2, 9}, /* round 4 */
};

/* i values just count up, so I'm not going to put them here. */

/* ==================================== */
/* Default values for each word in the  */
/* hash.                                */
/* ==================================== */

const word AA = 0x67452301;
const word BB = 0xefcdab89;
const word CC = 0x98badcfe;
const word DD = 0x10325476;



/*================================================================*/
/*   The T table of 64 32-bit integers as described in RFC 1321,  */
/*   where T[i] = (int)(4294967296 * abs(sin(i)))                 */
/*   (i is in radians for sin(i))                                 */
/*================================================================*/

/**
 * Values copied from
 * ftp://ftp.prenhall.com/pub/esm/computer_science.s-041/stallings/Tables/Crypto3e_PDF_Tables/T12-Vertical.pdf
 */
const word T[] = 
{
	0xd76aa478,	 /* T1 */
	0xe8c7b756,	 /* T2 */
	0x242070db,	 /* T3 */
	0xc1bdceee,	 /* T4 */
	0xf57c0faf,	 /* T5 */
	0x4787c62a,	 /* T6 */
	0xa8304613,	 /* T7 */
	0xfd469501,	 /* T8 */
	0x698098d8,	 /* T9 */
	0x8b44f7af,	 /* T10 */
	0xffff5bb1,	 /* T11 */
	0x895cd7be,	 /* T12 */
	0x6b901122,	 /* T13 */
	0xfd987193,	 /* T14 */
	0xa679438e,	 /* T15 */
	0x49b40821,	 /* T16 */
	0xf61e2562,	 /* T17 */
	0xc040b340,	 /* T18 */
	0x265e5a51,	 /* T19 */
	0xe9b6c7aa,	 /* T20 */
	0xd62f105d,	 /* T21 */
	0x02441453,	 /* T22 */
	0xd8a1e681,	 /* T23 */
	0xe7d3fbc8,	 /* T24 */
	0x21e1cde6,	 /* T25 */
	0xc33707d6,	 /* T26 */
	0xf4d50d87,	 /* T27 */
	0x455a14ed,	 /* T28 */
	0xa9e3e905,	 /* T29 */
	0xfcefa3f8,	 /* T30 */
	0x676f02d9,	 /* T31 */
	0x8d2a4c8a,	 /* T32 */
	0xfffa3942,	 /* T33 */
	0x8771f681,	 /* T34 */
	0x6d9d6122,	 /* T35 */
	0xfde5380c,	 /* T36 */
	0xa4beea44,	 /* T37 */
	0x4bdecfa9,	 /* T38 */
	0xf6bb4b60,	 /* T39 */
	0xbebfbc70,	 /* T40 */
	0x289b7ec6,	 /* T41 */
	0xeaa127fa,	 /* T42 */
	0xd4ef3085,	 /* T43 */
	0x04881d05,	 /* T44 */
	0xd9d4d039,	 /* T45 */
	0xe6db99e5,	 /* T46 */
	0x1fa27cf8,	 /* T47 */
	0xc4ac5665,	 /* T48 */
	0xf4292244,	 /* T49 */
	0x432aff97,	 /* T50 */
	0xab9423a7,	 /* T51 */
	0xfc93a039,	 /* T52 */
	0x655b59c3,	 /* T53 */
	0x8f0ccc92,	 /* T54 */
	0xffeff47d,	 /* T55 */
	0x85845dd1,	 /* T56 */
	0x6fa87e4f,	 /* T57 */
	0xfe2ce6e0,	 /* T58 */
	0xa3014314,	 /* T59 */
	0x4e0811a1,	 /* T60 */
	0xf7537e82,	 /* T61 */
	0xbd3af235,	 /* T62 */
	0x2ad7d2bb,	 /* T63 */
	0xeb86d391,	 /* T64 */
};


/* ==================================== */
/* MD5 functions for each round as      */
/* defined in the RFC                   */
/* ==================================== */

word F(word x, word y, word z);
word G(word x, word y, word z);
word H(word x, word y, word z);
word I(word x, word y, word z);

const md5_round_function round_functions[] = { &F, &G, &H, &I };



/* =========================================================== */
/* The MD5 hash process has been broken up as described in     */
/* RFC 1321:                                                   */
/* =========================================================== */

/* Step 1: Padding */
int perform_padding(byte* buffer, size_t original_message_length);

/* Step 2: Append Size in bits */
void append_size(byte* buffer, uint64_t original_message_size, int index);

/* Step 3: Initialize Buffer */
void initialize_md5_result(md5_result result);

/* Step 4: Process the message in 16-word chunks. */
void process(md5_chunk* message_buffer, size_t num_chunks, md5_result result);

/* Step 5: Output result as 32 hexadecimal digits. */
void output_hash(md5_result hash);


#endif  /* MD5_HASH_H */


