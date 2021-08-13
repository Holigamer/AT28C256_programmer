#include "helper_methods.h"
/**
 * Compare two arrays to each other
 */
bool array_cmp(byte *a, byte *b, int len_a, int len_b)
{
    int n;

    // if their lengths are different, return false
    if (len_a != len_b)
        return false;

    // test each element to be the same. if not, return false
    for (n = 0; n < len_a; n++)
        if (a[n] != b[n])
            return false;

    //ok, if we have not returned yet, they are equal :)
    return true;
}

// Split a String at a specific value
String getValue(String data, char separator, int index)
{
    int found = 0;
    int strIndex[] = {0, -1};
    int maxIndex = data.length() - 1;

    for (int i = 0; i <= maxIndex && found <= index; i++)
    {
        if (data.charAt(i) == separator || i == maxIndex)
        {
            found++;
            strIndex[0] = strIndex[1] + 1;
            strIndex[1] = (i == maxIndex) ? i + 1 : i;
        }
    }

    return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}
