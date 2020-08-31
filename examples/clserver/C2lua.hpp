#ifndef _c2lua_h_
#define _c2lua_h_

#include <luajit-2.1/lua.hpp>
#include "lunar.hpp"
#include "ssdbluachatserver.hpp"
#include "muduo/net/TcpConnection.h"

//class CppServer;
class C2lua{
    public:
        static const char className[];
        static Lunar<C2lua>::RegType methods[];
        
        C2lua(lua_State *L);
        ~C2lua();

        int handleLogout(lua_State *L);
        int sendMsg(lua_State *L);
        int isWsClient(lua_State *L);

    private:
        CppServer* server;

};
#endif
