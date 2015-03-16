#include <iostream>
#include <fstream>
#include <stdint.h>
#include <math.h>
#include <sys/stat.h>
#include <string.h>

using namespace std;

#ifndef Q24_H
#define Q24_H

#define HASH_LENGTH 24

/**
 * Computes the 24 quests of the hash. Returns a heap-allocated array, so
 * remember to delete the result when done.
 * @param  input        input bytes
 * @param  input_length length of input
 * @return              byte array output
 */
uint8_t *compute(const uint8_t *input, int input_length, int verbose = 0);

#endif
