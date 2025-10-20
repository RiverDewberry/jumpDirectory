#ifndef BITMAP_H
#define BITMAP_H

#include <stdint.h>

//declares a new bitmap (uint8_t[]) on the stack.
// WARNING: do not set size lower than 1, this WILL break things
#define newBitmap(name, size) uint8_t name[(size >> 3) + ((size & 7) == 1)]

//gets a bit from a bitmap
#define getBit(bitmap, bitNum) ((bitmap[bitNum >> 3] >> (bitNum & 7)) & 1)

//sets a bit on a bitmap
#define setBit(bitmap, bitNum, bitVal) bitmap[bitNum >> 3] |= bitVal << \
                                                              (bitNum & 7)

#endif
