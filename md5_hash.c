#include "md5_hash.h"

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>


/*
 * First round function.
 */
word F(word x, word y, word z)
{
    return (x & y) | (~x & z);
}

/*
 * Second round function.
 */
word G(word x, word y, word z)
{
    return (x & z) | (y & ~z);
}

/*
 * Third round function.
 */
word H(word x, word y, word z)
{
    return x ^ y ^ z;
}

/*
 * Fourth round function.
 */
word I(word x, word y, word z)
{
    return y ^ (x | ~z);
}

/**
 * Determine the size of the buffer that will be used to 
 * pad the message and append the message length as described
 * in RFC 1321 - Steps 1 - 2.
 *
 * The buffer must be big enough to meet all of the following
 * criteria with regard to bytes (instead of bits like the RFC
 * explains it):
 *
 * 1) Contain the entire message    (n bytes)
 * 2) Contain at least one extra byte of padding. (p + 1 bytes)
 * 3) Contain the size of the original message (8 bytes).
 * 4) n + p + 9 is a multiple of 64.
 *
 * This function will calculate and return the number of bytes
 * needed (i.e. the correct n + p + 9).
 */
size_t pad_buffer_size(size_t message_length)
{
    
    /* The + 9 accounts for the mandatory 1 byte of padding and  */
    /* the 8 bytes of size that will be appended.  */
    size_t additional = (MD5_CHUNK_LENGTH - ((message_length + 9) % MD5_CHUNK_LENGTH)) + 9;
    return message_length + additional;
}


/**
 * Pad the message_buffer as per RFC 1321.
 * The first bit after the first original_message_length bytes in the buffer
 * will be a 1, followed by as many 0s as it takes to reach
 * some multiple of MD5_CHUNK_LENGTH - 8 (The 8 is the space reserved
 * for storing  the size of the original_message.)
 *
 * This function will return the number of padded bytes added.
 */
int perform_padding(byte* message_buffer, size_t original_message_length)
{
    static const byte firstPad = 0x80;  /* 1000 0000 */
    static const byte padByte  = 0x00;  /* 0000 0000 */

    int added_bytes = 0;

    if (message_buffer != NULL)
    {

        int buffer_index = original_message_length;
        message_buffer[buffer_index++] = firstPad;
        size_t buffer_size = pad_buffer_size(original_message_length);

        while (buffer_index < buffer_size - sizeof(uint64_t))
        {
            message_buffer[buffer_index++] = padByte;
        }

        added_bytes = buffer_index - original_message_length;
    }
    return added_bytes;
}


/**
 * Insert the size of the original message to the buffer at the given index.
 * original_message_size is expected to be in bytes, and will be converted
 * into bits per RFC 1321.
 */
void append_size(byte* buffer, uint64_t original_message_size, int index)
{
    /* Accoring to the RFC:
     * (These bits are appended as two 32-bit words and 
     * appended low-order word first in accordance with the
     * previous conventions.)
     */
    byte* size_slot = (buffer + index);
    uint64_t size = BYTES_TO_BITS(original_message_size);
    if (buffer != NULL && index >= 0)
    {
        /* Make sure that the low-order bytes are first. */
        int shift = 0;
        while (shift < 64)
        {
            *size_slot = (byte)((size >> shift) & 0xFF);
            size_slot++;    /* move on to the next byte slot in the buffer. */
            shift += 8;     /* move on to the next octet of the size. */
        }
    }    
}


/**
 * Initialize the buffer with the default words specified in RFC 1321.
 */
void initialize_md5_result(md5_result result)
{
    result[A] = AA;
    result[B] = BB;
    result[C] = CC;
    result[D] = DD;
}

/**
 * Performs an MD5 operation as specified in the RFC:
 *
 * Let [abcd k s i] denote the operation
               a = b + ((a + F(b,c,d) + X[k] + T[i]) <<< s). 
 * 
 * The only difference is that in this function, abcd are the 
 * indices of the correct word in the result.
 */
static void perform_operation(int abcd[NUM_WORDS_RESULT], int k, int s, int i, 
                            md5_chunk X, md5_round_function function, md5_result result)
{
    int index_a = abcd[a];
    int index_b = abcd[b];
    int index_c = abcd[c];
    int index_d = abcd[d];

    word word_a = result[index_a];
    word word_b = result[index_b];
    word word_c = result[index_c];
    word word_d = result[index_d];

    word_a += function(word_b, word_c, word_d) + X[k] + T[i];
    word_a =  ROTATE_LEFT(word_a, s);
    word_a += word_b;

    result[index_a] = word_a;
}


/**
 * Helper function designed to perform the round operations for a chunk.
 * (The round number is called cs_round because it starts at 0, 
 * like how all good cs majors start indices at 0. :P)
 */
static void perform_round(md5_chunk chunk, int cs_round, md5_result result)
{
    int *abcd; 
    const int *k = k_values[cs_round];
    const int *s = shift_amounts[cs_round];
    md5_round_function function = round_functions[cs_round];

    int i = cs_round * NUM_WORDS_PER_CHUNK;
    int op_number;
    for (op_number = 0; op_number < NUM_WORDS_PER_CHUNK; op_number++, i++)
    {
        abcd = word_orders[op_number % NUM_WORDS_RESULT];
        perform_operation(abcd, 
                            k[op_number], 
                            s[op_number % NUM_WORDS_RESULT], 
                            i, 
                            chunk, 
                            function, 
                            result);
    }
}

/* Encodes input (words) into output (byte). 
 * Assumes len is a multiple of 4.
 *
 * Based off of the code from RFC 1321, this function
 * will make the low order bytes in each word come before
 * the high-order bytes.
 */
static void encode (md5_result hash)
{
    unsigned int i, j;
    const unsigned int len = NUM_WORDS_RESULT * sizeof(word); 

    /* Make a copy of what's currently in the buffer. */
    md5_result input;
    memcpy(input, hash, len);

    /* Look at the result in terms of bytes rather than words. */
    byte* output = (byte*)hash;

    /* Arrange the bytes in each word. */
    for (i = 0, j = 0; j < len; i++, j += sizeof(word))
    {
        /* Low-order bytes first. */
        output[j  ] = (byte)((input[i]      ) & 0xff);
        output[j+1] = (byte)((input[i] >>  8) & 0xff);
        output[j+2] = (byte)((input[i] >> 16) & 0xff);
        output[j+3] = (byte)((input[i] >> 24) & 0xff);
        /* High-order bytes last. */
    }
}

/* Decodes input (unsigned char) into output (UINT4). Assumes len is
 * a multiple of 4.
 */
static void decode (md5_chunk output)
{
    unsigned int i, j;
    const unsigned int len = NUM_WORDS_PER_CHUNK * sizeof(word);
    /* Make a copy of what's currently in the buffer. */
    byte input[len];
    memcpy(input, output, len);

    for (i = 0, j = 0; j < len; i++, j += 4)
    {
        output[i] = ((word)input[j]) | 
                    (((word)input[j+1]) << 8) |
                    (((word)input[j+2]) << 16) | 
                    (((word)input[j+3]) << 24);
    }
}

/**
 * Processes each md5_chunk with the MD5 algorithm and stores the result
 * in result.
 */
void process(md5_chunk* message_buffer, size_t num_chunks, md5_result result)
{
    int chunk;
    int round;
    word initialA, initialB, initialC, initialD;
    for (chunk = 0; chunk < num_chunks; chunk++)
    {
        decode(message_buffer[chunk]);

        initialA = result[A];
        initialB = result[B];
        initialC = result[C];
        initialD = result[D];

        for (round = 0; round < 4; round++)
        {
            perform_round(message_buffer[chunk], round, result); 
        }

        result[A] += initialA;
        result[B] += initialB;
        result[C] += initialC;
        result[D] += initialD;
    }

    encode(result);
}

/**
 * Copy the message (minus the null terminating character)
 * into the buffer.
 */
void copy_message(const char* const message, byte* buffer)
{
    memcpy(buffer, message, strlen(message));
}

/**
 * Print the hash out in hex!
 */
void output_hash(md5_result hash)
{
    printf("MD5 Hash: ");
    byte* result = (byte*) hash;
    int byteNum;
    for (byteNum = 0; byteNum < NUM_WORDS_RESULT * 4; byteNum++)
    {
        printf("%02x", result[byteNum]);
    }
    printf("\n");
}


/**
 * Create and print the MD5 hash of the given message.
 */
void md5_hash(const char* const message)
{
    md5_result hash;    /* 128-bit (16 byte) hash */

    byte* buffer = NULL;

    size_t current_buffer_size = 0;
    size_t padded_buffer_size;
    size_t message_length;
    int pad_amount;
    message_length = strlen(message);/*  + 1;//include \0 */
    
    /* Grow the buffer if needed for the MD5 process. */
    padded_buffer_size = pad_buffer_size(message_length);
    if (padded_buffer_size > current_buffer_size)
    {
        buffer = (byte*) realloc(buffer, padded_buffer_size);
    }

    /* Copy the data into the new buffer. */
    copy_message(message, buffer);

    /* Perform MD5 as described in RFC 1321: */
    
    /* Step 1: Pad. */
    pad_amount = perform_padding(buffer, message_length);

    /* Step 2: Append size. */
    append_size(buffer, message_length, padded_buffer_size - sizeof(uint64_t));

    /* Step 3: Initialize result with default values. */
    initialize_md5_result(hash);

    /* Step 4: Process the message. */
    process((md5_chunk*)buffer, padded_buffer_size/MD5_CHUNK_LENGTH, hash);

    /* Step 5: Output resulting hash in hex */
    output_hash(hash);

    free(buffer);
}

int main(int argc, char **argv)
{
    char* message;

    if (argc == 1)
    {
        /* Read text to hash from STDIN. */
        printf("Reading input to hash from console.\nSend an EOF (Ctrl+D on Linux and Ctrl+Z on Windows) when done.\n");

        /* The following has been adapted from here: */
        /* http://stackoverflow.com/questions/2496668/how-to-read-stdin-into-string-variable-until-eof-in-c */
        
        const int buffer_size = 1024;
        char buffer[buffer_size];
        size_t messageSize = 1; /*  includes NULL */
        /* Preallocate space.  We could just allocate one char here, 
         * but that wouldn't be efficient. */
        char *message = malloc(sizeof(char) * buffer_size);
        if(message == NULL)
        {
                perror("Failed to allocate message");
                exit(1);
        }
        message[0] = '\0'; /*  make null-terminated */

        while(fgets(buffer, buffer_size, stdin))
        {
            char *old = message;
            messageSize += strlen(buffer);
            message = realloc(message, messageSize);
            if(message == NULL)
            {
                perror("Failed to reallocate message");
                free(old);
                exit(2);
            }
            strcat(message, buffer);
        }

        if(ferror(stdin))
        {
            free(message);
            perror("Error reading from stdin.");
            exit(3);
        }

        md5_hash(message);
    }
    else
    {
        int arg;
        for (arg = 1; arg < argc; arg++)
        {
            message = argv[arg];
            md5_hash(message);
        }
    }

    return 0;
}

