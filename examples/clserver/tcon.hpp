/**
 *  * This is the main object that is actually used in the C++ code.
 *   * It is to be manipulated by Lua through the Lua wrapper object (which will
 *    * contain a pointer to this object).
 *     */
#ifndef _tcon_h_
#define _tcon_h_

// Notice that I don't need ANY Lua stuff in here...
#include <luajit-2.1/lua.hpp>

// I am using luna
#include "lunar.hpp"

#include "muduo/net/TcpConnection.h"
// The header file for the real C++ object
class TCON{
    public:
    static const char className[];
    static Lunar<TCON>::RegType methods[];

    TCON(lua_State *L);
    ~TCON();

    void setObject(lua_State *L);
    int sendMsg(lua_State *L);
    int getPeerAddr(lua_State *L);

    private:
    muduo::net::TcpConnection* real_con;
};
#endif

