#include "scodec.hpp"

int main(){
    std::string key = "中文的key";
    //string myString = "这是中文测试";
    string myString = "キーワードは半角スペースで区切って複数指定することができます。";
    std::string bitstr = Scodec::strtobit(myString);
    std::cout << bitstr << endl;
    std::string hexstr = Scodec::bittohex(bitstr);
    std::cout << hexstr << endl;
    std::string bitstr1 = Scodec::hextobit(hexstr);
    std::cout << "hex to bin:" << bitstr1 << std::endl; 
    std::cout << "bit to str:" << Scodec::bittostr(bitstr1) << std::endl;
    std::string hex1 = Scodec::strtohex(myString);
    std::cout << "str to hex:" << hex1 << std::endl;
    std::cout << "hex to str:" << Scodec::hextostr(hex1) << std::endl;
    std::string encodedstr;
    std::string encodedstr1;
    std::string encodedstr2;
    std::string orgstr;
    encodedstr = Scodec::encodeStr(myString,key);
    std::cout << "encode string:" << encodedstr << std::endl;
    std::cout << "bit encoded:" << Scodec::strtobit(encodedstr) << std::endl;
    std::string hexencode = Scodec::strtohex(encodedstr);
    std::cout << "hex encoded:" << hexencode << std::endl;
    std::string decodestrfromhex = Scodec::hextostr(hexencode);
    std::cout << "start decode hex to str:" << decodestrfromhex  << std::endl;
    encodedstr1 = Scodec::decodeStr(encodedstr,key);
    std::cout << "decode string:" << encodedstr1 << std::endl;
    std::cout << "bit:" << Scodec::strtobit(encodedstr1) << std::endl;
    std::string xorstr = Scodec::strxor(myString,key);
    std::cout << "xor:" << xorstr << std::endl;
    std::string xorstr1 = Scodec::strxor(xorstr,key);
    std::cout << "xor again:" << xorstr1 << std::endl;


    /*
    std::string encoded = Scodec::strxor(myString,key.c_str());
    std::cout << " str xor encode:" <<key << ",xor:" << encoded<< std::endl;
    std::string hex2 = Scodec::strtohex(encoded);
    std::cout << "str to hex:" << hex2 << std::endl;
    std::string encoded2 = Scodec::hextostr(hex2);
    std::cout << "hex to str(encoded):" << encoded2 << std::endl;
    std::cout << "xor again decode:" << Scodec::strxor(encoded2,key.c_str()) << std::endl;
    */

    
    
    
}
