#ifndef HLPR_METHODS
#define HLPR_METHODS

#include <Arduino.h>

bool array_cmp(byte *a, byte *b, int len_a, int len_b);
String getValue(String data, char separator, int index);
#endif