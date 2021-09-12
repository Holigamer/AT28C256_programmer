#ifndef FILEUTILS_H
#define FILEUTILS_H

#include <fstream>

using namespace std;

class FileUtils
{

public:
    /**
     * @brief This Method checks, if the given file is of the correct format.
     * @return false if all is well. 1 if the file has an error (not found or inaccessible) 2 if bytesize does not match parameter
     */
    static int checkFileForCompatibility(string fileName, int byteSize);

    /**
     * @brief Loads the bytes from the file into a buffer and prechecks the file for errors
     * @see checkFileForCompatibility(string, int)
     */ 
    static int loadIntoMemory(string fileName, char *buf, int bufSize);
};

#endif