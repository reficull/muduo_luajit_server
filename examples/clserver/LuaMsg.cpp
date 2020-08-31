#include "LuaMsg.h"
/*
LuaMsg::LuaMsg(const StringMessageCallback &cb):messageCallback_(cb)
{

};
*/
void LuaMsg::onMessage(const muduo::net::TcpConnectionPtr& conn,
    muduo::net::Buffer* buf,
    muduo::Timestamp receiveTime){
};

bool LuaMsg::ReadMsg(int packetID,std::string& ret){
    std::cout << "read msg:" << packetID << std::endl;
    return true; 
};

void LuaMsg::SendMsg(int packetID,std::string &ipPort, std::string &msg){
    std::cout << __FILE__ << ":" << __LINE__ <<  ",send msg to :" << ipPort << ", msg:" << msg << std::endl;
};
