
#include <muduo/base/Logging.h>
#include <muduo/base/Mutex.h>
#include <muduo/net/EventLoopThread.h>
#include <muduo/net/TcpClient.h>

#include <boost/bind.hpp>
#include <boost/noncopyable.hpp>

#include <iostream>
#include <stdio.h>
#include "strxor.hpp"
using namespace muduo;
using namespace muduo::net;

char key[5] = {'m','n','t','e','c'};

class ChatClient : boost::noncopyable
{
 public:
  ChatClient(EventLoop* loop, const InetAddress& serverAddr)
    : client_(loop, serverAddr, "ChatClient")
  {
    client_.setConnectionCallback(
        boost::bind(&ChatClient::onConnection, this, _1));
    client_.setMessageCallback(
        boost::bind(&ChatClient::onMessage,this,  _1, _2, _3));
    client_.enableRetry();
  }

  void connect()
  {
    client_.connect();
  }


  void disconnect()
  {
    client_.disconnect();
  }

  void write(const StringPiece& message)
  {
/*    MutexLockGuard lock(mutex_);
    if (connection_)
    {
      connection_.send(message);
    }
    */
      //printf("sending:%s,encoded:%s\n",message.c_str());

      client_.send(encryptDecryp(tmessage,key));
  }

 private:
  void onConnection(const TcpConnectionPtr& conn)
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

  void onMessage(const TcpConnectionPtr&,
                       const string& message,
                       Timestamp)
  {
        printf("<<< %s\n", message.c_str());
        //printf("decoded <<< %s\n",encryptDecrypt( message,key).c_str());
  }

  TcpClient client_;
  MutexLock mutex_;
  TcpConnectionPtr connection_;
};

int main(int argc, char* argv[])
{
  LOG_INFO << "pid = " << getpid();
  if (argc > 2)
  {
    EventLoopThread loopThread;
    uint16_t port = static_cast<uint16_t>(atoi(argv[2]));
    InetAddress serverAddr(argv[1], port);

    ChatClient client(loopThread.startLoop(), serverAddr);
    client.connect();
    std::string line;
    while (std::getline(std::cin, line))
    {
      client.write(line);
    }
    client.disconnect();
    CurrentThread::sleepUsec(1000*1000);  // wait for disconnect, see ace/logging/client.cc
  }
  else
  {
    printf("Usage: %s host_ip port\n", argv[0]);
  }
}

