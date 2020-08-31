#include <iostream>
#include <muduo/base/Logging.h>
#include <muduo/net/Buffer.h>
#include <muduo/net/Endian.h>
#include <muduo/net/TcpConnection.h>

#include <boost/function.hpp>
#include <boost/noncopyable.hpp>

#define lua_userdata_cast(L, pos, T) static_cast<T*>(luaL_checkudata((L), (pos), #T))

using namespace std;
using namespace muduo;
using namespace muduo::net;
class LuaMsg : boost::noncopyable
{
    public:
        typedef boost::function<void (const muduo::net::TcpConnectionPtr&,
                const muduo::string& message,
                muduo::Timestamp)> StringMessageCallback;
        explicit LuaMsg(const StringMessageCallback &cb):messageCallback_(cb){
            cout << __FILE__ << ":" << __LINE__ << ":lua msg init with cb var." << endl; 
        };
        LuaMsg(){
            cout << __FILE__ << ":" << __LINE__ << ":lua msg init." << endl; 
        };

  void onMessage(const muduo::net::TcpConnectionPtr& conn,
                 muduo::net::Buffer* buf,
                 muduo::Timestamp receiveTime);
  
        std::string LuaScriptRoot;
        static bool ReadMsg(int packetID,const char* ret);
        void SendMsgByFD();
        static void SendMsg(int packetID,const char* ipPort,const char* msg);
        void UserBroadcast();
        void ZoneBroadcast();
        void SceneBroadcast();
        void WorldBroadcast();

    private:
        StringMessageCallback messageCallback_;
};
bool LuaMsg::ReadMsg(int packetID,const char* ret){
    std::cout << "read msg:" << packetID << std::endl;
    return true; 
};

void LuaMsg::SendMsg(int packetID,const char* ipPort, const char* msg){
    std::cout << "in c++: send msg to :" << ipPort << ", msg:" << msg << std::endl;
    //const_cast<TcpConnection*>( static_cast<const TcpConnection*>(conn))->send(msg);
};
extern "C" {
    LuaMsg* CLuaMsg_new(){
        return new LuaMsg;
    }
    void CLuaMsg_gc(LuaMsg* this_){
        delete this_;
    }
    void CLuaMsg_ReadMsg(LuaMsg* this_,int packetID,const char* ret)
    {
        this_->ReadMsg(packetID,ret);
    }

    void CLuaMsg_SendMsg(LuaMsg* this_,int packetID,const char* ipPort,const char* msg)
    {
        this_->SendMsg(packetID,ipPort, msg);
    }
}
