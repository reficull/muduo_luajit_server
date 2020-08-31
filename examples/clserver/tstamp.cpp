#include "tstamp.hpp"

TSTAMP::TSTAMP(lua_State *L){
    real_tstamp = static_cast<muduo::Timestamp*>(lua_touserdata(L,1));
}

int TSTAMP::now(lua_State *L){
    lua_pushstring(L,muduo::Timestamp::now().toString().c_str());
    return 1;
}

int TSTAMP::addTime(lua_State *L){
    double seconds = lua_tonumber(L,1);
    std::string t = muduo::addTime(muduo::Timestamp::now(),seconds).toString();
    lua_pushstring(L,t.c_str());
    return 1;
}

TSTAMP::~TSTAMP(){
}

const char TSTAMP::className[] = "TSTAMP";
Lunar<TSTAMP>::RegType TSTAMP::methods[] ={
    LUNAR_DECLARE_METHOD(TSTAMP,now),
    LUNAR_DECLARE_METHOD(TSTAMP,addTime),
    {0,0}
};
