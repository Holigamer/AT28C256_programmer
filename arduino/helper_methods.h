#ifndef HLPR_METHODS
#define HLPR_METHODS

#include <Arduino.h>

/**
 * @brief Compare if two arrays are equal
 */
bool array_cmp(byte *a, byte *b, int len_a, int len_b);
/**
 * @brief Substring at separator, returns the n-th (index) entry starting with index=0.
 */
String getValue(String data, char separator, int index);
#endif