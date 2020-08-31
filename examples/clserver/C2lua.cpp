#include "C2lua.hpp"
#include <string>


C2lua::C2lua(lua_State *L){
    server =static_cast<CppServer*>(lua_touserdata(L,1));
}

int C2lua::handleLogout(lua_State *L){
    std::cout << "C2lua side handleLogout called\n" << std::endl;
    muduo::net::TcpConnectionPtr* real_con = static_cast<muduo::net::TcpConnectionPtr*>(lua_touserdata(L,1));
    std::string reason(lua_tostring(L,2));
    std::cout << "disconnect reason:" << reason << std::endl;
    //std::string t("test");
    server->handleLogout(real_con,reason);
    return 1;
}

int C2lua::sendMsg(lua_State *L){
    std::cout << "inside c2lua:sendmsg" << std::endl;
    muduo::net::TcpConnectionPtr* real_con = static_cast<muduo::net::TcpConnectionPtr*>(lua_touserdata(L,1));
    if(NULL == *real_con){
        std::cout << "C2lua::sendMsg pointer not TcpConnectionPtr,or disconnected already." << std::endl;
    }else{
        MLOG_INFO << "make msg:" ;
        std::string msg(lua_tostring(L,2));
        MLOG_INFO << "sending msg in c2lua:" << real_con  ;
        if(msg.length()>1)
            try{
                server->lsendMsg(real_con,msg);
                MLOG_INFO << __FILE__ << ":" << __LINE__ <<  ": send complete" ;
            }catch(unsigned char* em){
                MLOG_INFO << " send error:" << em;
            }
    }
    return 1;
}

int C2lua::isWsClient(lua_State *L){
    muduo::net::TcpConnectionPtr* real_con = static_cast<muduo::net::TcpConnectionPtr*>(lua_touserdata(L,1));
    return server->isWsClient(real_con); 
}

C2lua::~C2lua(){

}

const char C2lua::className[] = "C2lua";
Lunar<C2lua>::RegType C2lua::methods[]={
    LUNAR_DECLARE_METHOD(C2lua,handleLogout),
    LUNAR_DECLARE_METHOD(C2lua,sendMsg),
    LUNAR_DECLARE_METHOD(C2lua,isWsClient),
    {0,0}
};
