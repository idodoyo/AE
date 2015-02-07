#ifndef DATA_TYPE_HEADER_FILE
#define DATA_TYPE_HEADER_FILE

#include <string>
#include <map>
#include <list>
#include <vector>
#include <fstream>
#include <sstream>


using std::map;
using std::string;
using std::vector;
using std::list;
using std::ofstream;
using std::ifstream;
using std::istringstream;
using std::ostringstream;

typedef enum { HSFalse, HSTrue } HSBoolValue;
typedef int HSBool;

typedef byte HSByte;

typedef char HSChar;
typedef unsigned char HSUChar;
typedef const char HSCChar;

typedef char HSInt8;
typedef unsigned char HSUInt8;

typedef short HSShort;
typedef unsigned short HSUShort;

typedef int HSInt;
typedef unsigned int HSUInt;

typedef __int64 HSInt64;
typedef unsigned __int64 HSUInt64;

typedef long HSLong;
typedef unsigned long HSULong;

typedef float HSFloat;
typedef double HSDouble;

typedef void HSVoid;

typedef std::string HSString;


#endif