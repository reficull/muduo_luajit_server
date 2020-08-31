#ifndef LUAMSG_H
#define LUAMSG_H 
#include <iostream>
#include <muduo/base/Logging.h>
#include <muduo/net/Buffer.h>
#include <muduo/net/Endian.h>
#include <muduo/net/TcpConnection.h>

#include <boost/function.hpp>
#include <boost/noncopyable.hpp>
using namespace std;
using namespace muduo;
using namespace muduo::net;
class LuaMsg : boost::noncopyable
{
    public:
        typedef boost::function<void (const muduo::net::TcpConnectionPtr&,
                const muduo::string& message,
                muduo::Timestamp)> StringMessageCallback;
        explicit LuaMsg(const StringMessageCallback &cb):messageCallback_(cb){};
        LuaMsg(){
            cout << "lua msg first run" << endl; 
        };

  void onMessage(const muduo::net::TcpConnectionPtr& conn,
                 muduo::net::Buffer* buf,
                 muduo::Timestamp receiveTime);
  
        std::string LuaScriptRoot;
        static bool ReadMsg(int packetID,std::string &ret);
        void SendMsgByFD();
        static void SendMsg(int packetID,std::string &conn,std::string &msg);
        void UserBroadcast();
        void ZoneBroadcast();
        void SceneBroadcast();
        void WorldBroadcast();

    private:
        StringMessageCallback messageCallback_;
};
#endif
