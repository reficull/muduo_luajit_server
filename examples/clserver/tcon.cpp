#include "tcon.hpp"
#include "strxor.hpp"

TCON::TCON(lua_State *L){
    real_con =static_cast<muduo::net::TcpConnection*>(lua_touserdata(L,1));
}

void TCON::setObject(lua_State *L){
    real_con = static_cast<muduo::net::TcpConnection*>(lua_touserdata(L,1));
}

int TCON::sendMsg(lua_State *L){

    //char mntecsecret[] = {'m','n','t','e','c'};
    std::cout << __FILE__ << ":" << __LINE__ << ": sendMsg in tcon" << std::endl;
    real_con->send(std::string(lua_tostring(L,1)));
    std::cout << __FILE__ << ":" << __LINE__ << ": after sendMsg in tcon" << std::endl;
    //real_con->send(encryptDecrypt(std::string(lua_tostring(L,1)),mntecsecret));
    return 1;
}

int TCON::getPeerAddr(lua_State *L){
    lua_pushstring(L,real_con->peerAddress().toIpPort().c_str());
    return 1;
}

TCON::~TCON(){

}


const char TCON::className[] = "TCON";
Lunar<TCON>::RegType TCON::methods[] ={
    LUNAR_DECLARE_METHOD(TCON,sendMsg),
    LUNAR_DECLARE_METHOD(TCON,getPeerAddr),
    {0,0}
};
