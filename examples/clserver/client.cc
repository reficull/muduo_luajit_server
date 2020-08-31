#include "codec.h"
#include "client.hpp"

int main(int argc, char* argv[])
{
  uint16_t port; 
  InetAddress serverAddr;
  LOG_INFO << "pid = " << getpid();
  if (argc > 2)
  {
    port = static_cast<uint16_t>(atoi(argv[2]));
    serverAddr = InetAddress(argv[1], port);
  }else{
    port = 40010;
    serverAddr = InetAddress("localhost",port);
  }
    EventLoopThread loopThread;
    ChatClient client(loopThread.startLoop(), serverAddr);
    client.connect();
    client.loadLua(1);
    std::string line;
    while (std::getline(std::cin, line))
    {
        boost::algorithm::trim_left(line);
            //std::vector<std::string> v;
        if(client.connection_->connected())    
            script::Lua::call(client.L,"msgSend",static_cast<void*>(&(*(client.connection_))),100,line.c_str());
        //client.write(line);
    }
    client.disconnect();
    CurrentThread::sleepUsec(1000*1000);  // wait for disconnect, see ace/logging/client.cc
  
/*  
  {
  else
  {
    printf("Usage: %s host_ip port\n", argv[0]);
  }
  */
}

