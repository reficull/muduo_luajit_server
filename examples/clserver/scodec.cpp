#include "scodec.hpp"
#define SETWIDTH 8 
namespace Scodec{
std::string  strtobit(std::string& str){
    std::stringstream ss; 
    for (std::size_t i = 0; i < str.size(); ++i)
    {
        ss << bitset<8>(str.c_str()[i]);
        //std::cout << "char:" << str.c_str()[i] << ",bit:" << bitset<8>(str.c_str()[i]) << ",ascii:" << (int)(str.c_str()[i]) << std::endl;
    }
    return ss.str();
}

std::string bittohex(std::string& str){
    std::stringstream ss; 
    for(std::size_t i = 0;i<str.size();i+=SETWIDTH){
        
        //std::cout << i << "-" << i+SETWIDTH << ":" <<  str.substr(i,SETWIDTH) << std::endl;
        bitset<SETWIDTH> set(str.substr(i,SETWIDTH));
        ss << hex << set.to_ulong();
    }
    return ss.str();
}

std::string bittostr(std::string& str){
    std::string res;
    for(std::size_t i = 0;i<str.size();i+=SETWIDTH){
        bitset<SETWIDTH> set(str.substr(i,SETWIDTH));
        int tb = static_cast<int>(set.to_ulong());
        char tc = static_cast<char>(tb);
        //std::cout << "bit:" << set.to_string() << ",dec:" << tc << std::endl;
        res += tc;
    } 
    return res;
}

std::string hexchartobit(char c)
{
    std::string res = "";
    switch(toupper(c))
    {
        case '0': res = "0000";break;
        case '1': res = "0001";break;
        case '2': res = "0010";break;
        case '3': res = "0011";break;
        case '4': res = "0100";break;
        case '5': res = "0101";break;
        case '6': res = "0110";break;
        case '7': res = "0111";break;
        case '8': res = "1000";break;
        case '9': res = "1001";break;
        case 'A': res = "1010";break;
        case 'B': res = "1011";break;
        case 'C': res = "1100";break;
        case 'D': res = "1101";break;
        case 'E': res = "1110";break;
        case 'F': res = "1111";break;
                      
    }
    return res;
}

std::string hextobit(std::string& str){
    std::string bin;
    for(std::size_t i = 0;i < str.size();++i){
        bin += hexchartobit(str[i]);
    }
    return bin;
}


std::string strxor(std::string& toCodec,const char* k) {
    std::string output = toCodec;

    for (unsigned int i = 0; i < toCodec.size(); i++){
        output[i] = toCodec[i] ^ k[i % (sizeof(k) / sizeof(char))];
        //std::cout << "i:" << i << ",char:" << toCodec[i] << ",encoded:" << output[i] << std::endl;
    }
    return output;
}

std::string strxor(std::string& toCodec,std::string& k) {
    return strxor(toCodec,k.c_str());
}


std::string strtohex(std::string& str){
    std::string t1 = strtobit(str);
    std::string res = bittohex(t1);
    return res;
}

std::string hextostr(std::string& str){
    std::string t1 = hextobit(str);
    std::string res = bittostr(t1);
    return res;
}


std::string encodeStr(std::string str,std::string key){
    unsigned char* encstr = static_cast<unsigned char*>(malloc(sizeof(unsigned char) * str.size() ));
    int m_nIdx = 0;
    unsigned char c,k,c2;
    for(size_t i=0;i<str.size();++i){
        //    encstr[i] = Encode(str.c_str()[i]);
        c = str.c_str()[i];
        if(static_cast<long unsigned int>(m_nIdx) >= key.size())
        {
            m_nIdx = 0;
        }

        k = key.c_str()[m_nIdx];
        ++m_nIdx;

        c2 = static_cast<unsigned char>(k + c);
        //printf("num of k+c:%d,hex:%x\n" , c2,c2 );
        encstr[i] = c2;
    }
    const char* tstr = reinterpret_cast<const char*>(encstr);
    std::string ret = std::string(tstr,str.size());
    //std::cout << "sizeof encstr:" << sizeof(tstr) << ",str size:" << ret.size() << std::endl;
    return ret;
}

const char* encodeStr(char* str,const char* key){
    std::string ret = encodeStr(std::string(str),std::string(key));
    return ret.c_str(); 
}
std::string decodeStr(std::string str,std::string key){
    std::string decodedstr;
    int m_nIdx = 0;
    unsigned char c,k,c2;
    for(size_t i=0;i<str.size();++i){
        std::stringstream ss;

        c = str.c_str()[i];
        if(static_cast<long unsigned int>(m_nIdx) >= key.size())
        {
            m_nIdx = 0;
        }
        k = key.c_str()[m_nIdx];
        ++m_nIdx;

        c2 = static_cast<unsigned char>(c - k);
        ss << c2;
        decodedstr +=ss.str();
    }
    return decodedstr;
}
}

extern "C" {
    const char* Scodec_encode(char* str,const char* key){
        return Scodec::encodeStr(str,key);
    }
}
