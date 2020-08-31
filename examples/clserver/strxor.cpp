#include "strxor.hpp"

std::string encryptDecrypt(std::string toCodec,char* k) {
    std::string output = toCodec;
    int sKe = sizeof(k);
    int sChar = sizeof(char);
    int num = sKe/sChar;

    for (unsigned int i = 0; i < toCodec.size(); i++)
        output[i] = toCodec[i] ^ k[i % num];
        //output[i] = toCodec[i] ^ k[i % (sizeof(k) / sizeof(char))];

    return output;
}
