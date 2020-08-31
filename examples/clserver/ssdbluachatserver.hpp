#ifndef __SSDBLUACHATSERVER_
#define __SSDBLUACHATSERVER_

#include <luajit-2.1/lua.hpp>
#include <memory>
#include <errno.h>
#include <LuaBridge/LuaBridge.h>
#include "lua_utility.hpp"
#include <fcntl.h>
#include <sys/stat.h> 
#include <sys/types.h> 
#include <muduo/base/Atomic.h>
#include <muduo/base/Logging.h>
#include <muduo/base/Thread.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/InetAddress.h>
#include <muduo/net/TcpServer.h>
#include <boost/bind.hpp>
#include <boost/iostreams/filtering_streambuf.hpp>
#include <boost/iostreams/copy.hpp>
#include <boost/iostreams/filter/gzip.hpp>
#include <boost/algorithm/string.hpp>
//#include <boost/vmd/tuple.hpp> 
#include <boost/multi_array.hpp> 
#include <set>
#include <map>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include <utility>
#include <unistd.h>
#include <sstream>
#include <fstream>
#include <iostream>
#include <pcre++.h>
#include <yaml-cpp/yaml.h>
#include <time.h>
#include <memory>
#include <libconfig.h++>
//#include "LuaMsg.h"
#include "define.h"
#include "map.hpp"
#include "strxor.hpp"
#include <Tmx.h>
#include <msgpack.hpp>
#include "WebSocket/WebSocket.h"
//#include "RakNetSocket2.h"

//the same as lua script packet id
#define GC_BROADCAST 10103
#define GC_DISCONNECT 10105
#define TYPE_GZIP 1
using namespace std;
using namespace muduo;
using namespace muduo::net;
using namespace luabridge;

namespace ph = std::placeholders;
enum wsStatus{
    WSCONNECTED=1
};

enum MSGTYPE{
    MT_NORMAL,MT_XOR,MT_WEBSOCKET
};

class CppServer
{
    private:
        lua_State *L;
        TcpServer server_;
        AtomicInt64 transferred_;
        AtomicInt64 receivedMessages_;
        int64_t oldCounter_;
        Timestamp startTime_;
        typedef std::set<TcpConnectionPtr> ConnectionList;
        ConnectionList connections_;
        typedef std::map<std::string,TcpConnectionPtr> UserIdConList;
        UserIdConList userIdCon;
        typedef std::map<TcpConnectionPtr,int> msgUserList;
        msgUserList msgUList;
        std::string accessTime;
        std::string cwdpath;
        std::string mapFileName;
        Tmx::Map *map;
        WebSocket ws; 
        void doLogout(const TcpConnectionPtr& conn);
        void onConnection(const TcpConnectionPtr& conn);
        void onMessage(const TcpConnectionPtr& conn, Buffer* buf, Timestamp);
        void msghdl(const TcpConnectionPtr& conn,char *msg);
        void retMsg(std::vector<std::string>& v,const TcpConnectionPtr& conn,int packetID);
        void setAccessTime();
        void checkRenew();
        void cronJob();
        void printThroughput();
        void cleanup(const TcpConnectionPtr& conn);

    public:
        CppServer(EventLoop* loop, const InetAddress& listenAddr,int numThreads)
            : server_(loop, listenAddr, "chatServer"),
            oldCounter_(0),
            startTime_(Timestamp::now())
    {
        server_.setConnectionCallback(
                std::bind(&CppServer::onConnection, this, ph::_1));
        server_.setMessageCallback(
                std::bind(&CppServer::onMessage, this, ph::_1, ph::_2, ph::_3));
        server_.setThreadNum(numThreads);
        //loop->runEvery(10.0, boost::bind(&CppServer::printThroughput, this));
        loop->runEvery(1.0, boost::bind(&CppServer::checkRenew, this));
        loop->runEvery(0.1, boost::bind(&CppServer::cronJob, this));

    }

        void loadConfig();


        void loadLua(int firstTime=0);
        void loadMap();

        void start(std::string &cwdpath);
        /*
        ssdb::Client* reconnectDB()
        {
            ssdb= ssdb::Client::connect(ssdbIP, ssdbPort);
            if(ssdb== NULL){
                cout << "fail to connect to ssdb server!\n" << endl;
                return 0;
            }
            cout << "ssdb connected" << endl;
            return ssdb;
        }
        */
        void handleLogout(const TcpConnectionPtr* conn,std::string& reason);
        void lsendMsg(const TcpConnectionPtr* conn,std::string& msg);
        bool isWsClient(const TcpConnectionPtr* conn);

};
/*
   char the_path[256];
   getcwd(the_path, 255);
   std::string cwdpath = std::string(the_path);
   std::string LuaMsg::LUA_SCRIPT_ROOT = cwdpath;
   */


#endif
