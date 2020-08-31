#ifndef CLIENT_H 
#define CLIENT_H 

#include <stdio.h>
#include <sstream>
#include <string>
#include <vector>
#include <iostream>
#include <bitset>

#include <boost/function.hpp>
#include <boost/noncopyable.hpp>
#include <boost/iostreams/filtering_streambuf.hpp>
#include <boost/iostreams/copy.hpp>
#include <boost/iostreams/filter/gzip.hpp>
#include <boost/bind.hpp>
#include <boost/algorithm/string.hpp>

#include <muduo/base/Logging.h>
#include <muduo/base/StringPiece.h>
#include <muduo/net/Buffer.h>
#include <muduo/net/Endian.h>
#include <muduo/net/TcpConnection.h>
#include <muduo/base/Logging.h>
#include <muduo/base/Mutex.h>
#include <muduo/net/EventLoopThread.h>
#include <muduo/net/TcpClient.h>


#include <muduo/base/Thread.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/EventLoopThreadPool.h>
#include <muduo/net/InetAddress.h>

#include <utility>

#include <stdio.h>
#include <unistd.h>
#include <luajit-2.1/lua.hpp>
#include "lua_utility.hpp"
#include "tcon.hpp"
#include "strxor.hpp"
//#include "codec.hpp"

#define MLOG_INFO if (muduo::Logger::logLevel() <= muduo::Logger::INFO) \
                                                   muduo::Logger(__FILE__, __LINE__).stream()
using namespace muduo;
using namespace muduo::net;
namespace ph = std::placeholders;

//using std::placeholders::_1;
//using std::placeholders::_2;
//using std::placeholders::_3;

class ChatClient : boost::noncopyable
{
    public:
        lua_State *L;
        ChatClient(EventLoop* loop, const InetAddress& serverAddr)
            : client_(loop, serverAddr, "ChatClient")
            //codec_(std:bind(&ChatClient::onStringMessage, this, _1, _2, _3))
    {
        client_.setConnectionCallback(
                std::bind(&ChatClient::onConnection, this, ph::_1));
        client_.setMessageCallback(
               // boost::bind(&LengthHeaderCodec::onMessage, &codec_, _1, _2, _3));
               std::bind(&ChatClient::onMessage,this,ph::_1,ph::_2,ph::_3));
        client_.enableRetry();
    }
        void connect();


        void disconnect();

        void write(const muduo::StringPiece& message);
        void loadLua(int firstTime);
        TcpConnectionPtr connection_;
    private:
        void onConnection(const TcpConnectionPtr& conn);
        void onMessage(const muduo::net::TcpConnectionPtr& conn,muduo::net::Buffer* buf,muduo::Timestamp receiveTime);
        /*
        void onStringMessage(const TcpConnectionPtr&,
                const string& message,
                Timestamp);
        */
        TcpClient client_;
        //LengthHeaderCodec* codec_;
        muduo::MutexLock mutex_;
};

#endif 
