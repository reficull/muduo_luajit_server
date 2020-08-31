#include <string>
#include <bitset>
#include <iostream>
#include <sstream>
using namespace std;

namespace Scodec{
    std::string strtobit(std::string& str);
    std::string bittohex(std::string& str);
    std::string hextobit(std::string& str);
    std::string bittostr(std::string& str);
    std::string strxor(std::string& toCodec,const char* k);
    std::string strxor(std::string& toCodec,std::string& k);

    std::string strtohex(std::string& str);
    std::string hextostr(std::string& str);
    std::string encodeStr(std::string str,std::string key);
    const char* encodeStr(char* str,const char* key);
    std::string decodeStr(std::string str,std::string key);
};


