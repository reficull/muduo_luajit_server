#include "client.hpp"
#include "tcon.hpp"

void ChatClient::connect()
{
    ChatClient::client_.connect();
}


void ChatClient::disconnect()
{
    ChatClient::client_.disconnect();
}

void ChatClient::write(const muduo::StringPiece& message)
{
    MutexLockGuard lock(mutex_);
    if (connection_)
    {
        connection_->send( message);
    }
}

void ChatClient::onConnection(const TcpConnectionPtr& conn)
{
    LOG_INFO << conn->localAddress().toIpPort() << " -> "
        << conn->peerAddress().toIpPort() << " is "
        << (conn->connected() ? "UP" : "DOWN");

    MutexLockGuard lock(mutex_);
    if (conn->connected())
    {
        connection_ = conn;
    }
    else
    {
        connection_.reset();
    }
}

void ChatClient::onMessage(const muduo::net::TcpConnectionPtr& conn,muduo::net::Buffer* buf,muduo::Timestamp receiveTime)
{
    std::string str = buf->retrieveAllAsString();
    if(str.length()<1) return;
//    MLOG_INFO << "onMessage:" << str;
            //std::vector<std::string> v;
            script::Lua::call(L,"msgHandle",static_cast<void*>(&(*conn)),100,str.c_str());

}
void ChatClient::loadLua(int firstTime)
{
    //getGlobalNamespace(L).beginNamespace("DB").addCFunction("hset",&CppServer::hset).endNamespace();

    // run the Lua script
    L = luaL_newstate();
    luaL_openlibs(L);
    std::string spath = "scripts/client.lua";
    if(firstTime){
        Lunar<TCON>::Register(L);
        std::cout << "cwd:" << spath << std::endl;
        if(luaL_dofile(L, spath.c_str())){
            std::cout << "ERROR:" << lua_tostring(L,-1) <<  ",client.lua not loaded,check the code" << std::endl;    
            //exit(0);
        }else{
            std::cout << "--------lua loaded---" << std::endl;
            //printf("this pointer %p,\n",this);
            //script::Lua::call(L,"passCobj",(void*)&(*this));
        }
    }else{
        if(luaL_dofile(L, spath.c_str())){
            std::cout << "ERROR:" << lua_tostring(L,-1) <<  ",client.lua not loaded,check the code" << std::endl;    
            //exit(0);
        }else{
            std::cout << "--------lua reloaded---" << std::endl;
        }

    }
}


/*
void ChatClient::onStringMessage(const TcpConnectionPtr&,
        const string& message,
        Timestamp)
{
    printf("<<< %s\n", message.c_str());
}
*/

