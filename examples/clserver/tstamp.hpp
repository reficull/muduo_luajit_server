#ifndef _tstamp_h_
#define _tstamp_h_
#include <luajit-2.1/lua.hpp>

// I am using luna
#include "lunar.hpp"

#include "muduo/base/Timestamp.h"

class TSTAMP{
    public:
    static const char className[];
    static Lunar<TSTAMP>::RegType methods[];


    TSTAMP(lua_State *L);
    ~TSTAMP();
    int now(lua_State *L);
    int addTime(lua_State *L);

    private:
    muduo::Timestamp* real_tstamp; 
};

#endif
