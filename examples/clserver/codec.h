#ifndef MUDUO_EXAMPLES_ASIO_CHAT_CODEC_H
#define MUDUO_EXAMPLES_ASIO_CHAT_CODEC_H

#include <muduo/base/Logging.h>
#include <muduo/net/Buffer.h>
#include <muduo/net/Endian.h>
#include <muduo/net/TcpConnection.h>

#include <boost/function.hpp>
#include <boost/noncopyable.hpp>
#include <boost/iostreams/filtering_streambuf.hpp>
#include <boost/iostreams/copy.hpp>
#include <boost/iostreams/filter/gzip.hpp>
#include <sstream>
#include <string>
#include <vector>
#include <iostream>
#include <bitset>
#include "strxor.hpp"
#include <luajit-2.1/lua.hpp>
#include "lua_utility.hpp"

//char mntecsecret[] = {'m','n','t','e','c'};
class LengthHeaderCodec : boost::noncopyable
{
    public:
        lua_State *L;
        typedef boost::function<void (const muduo::net::TcpConnectionPtr&,
                const muduo::string& message,
                muduo::Timestamp)> StringMessageCallback;

        explicit LengthHeaderCodec(const StringMessageCallback& cb)
            : messageCallback_(cb)
        {
        }

        void onMessage(const muduo::net::TcpConnectionPtr& conn,
                muduo::net::Buffer* buf,
                muduo::Timestamp receiveTime)
        {
            //std::string str = encryptDecrypt(buf->retrieveAllAsString(),mntecsecret);
            std::string str = buf->retrieveAllAsString();
            std::vector<char> bytes(str.begin(),str.end());
            unsigned char b1 = bytes[0];
            unsigned char b2 = bytes[1];
            /*
            std::cout << "b1 and b2:  "  << std::endl;
            printf("0x%08x,0x%08x\n",b1,b2);
            std::bitset<8> b1x(b1);
            std::bitset<8> b2x(b2);
            std::cout << b1x << "," << b2x << std::endl;
            printf("0x%08x,0x%08x\n",31,139);
            std::bitset<8> g1x(31);
            std::bitset<8> g2x(139);
            std::cout << g1x << "," << g2x << std::endl;
            */
            std::vector<std::string> v;
            v = script::Lua::getRet(L,"msgHandle",static_cast<void*>(&(*conn)),100,str.c_str());
            if((b1 == 0x1f) && (b2 == 0x8b)){
            //if((b1 == 0x72) && (b2 == 0xe5)){
                std::cout << "gzipped " << std::endl;
                            std::stringstream ss(str);
                            boost::iostreams::filtering_streambuf<boost::iostreams::input> inbuf;
                            inbuf.push(boost::iostreams::gzip_decompressor());
                            inbuf.push(ss);
                            std::stringstream ostr;
                            boost::iostreams::copy(inbuf,ostr);
                            std::cout << "unzipped:" <<  ostr.str() << std::endl;
            }else{
                std::cout << "not gzip" << std::endl;
                std::cout <<   str  << std::endl;
            }
            //std::cout << "decode:" <<   buf->retrieveAllAsString() << std::endl;
            /*
               while (buf->readableBytes() >= kHeaderLen) // kHeaderLen == 4
               {
            // FIXME: use Buffer::peekInt32()
            const void* data = buf->peek();
            int32_t be32 = *static_cast<const int32_t*>(data); // SIGBUS
            const int32_t len = muduo::net::sockets::networkToHost32(be32);
            if (len > 65536 || len < 0)
            {
            LOG_ERROR << "Invalid length " << len;
            conn->shutdown();  // FIXME: disable reading
            break;

            }
            else if (buf->readableBytes() >= len + kHeaderLen)
            {
            buf->retrieve(kHeaderLen);
            muduo::string message(buf->peek(), len);
            messageCallback_(conn, message, receiveTime);
            buf->retrieve(len);
            }
            else
            {
            break;
            }
            }
            */
        }

        // FIXME: TcpConnectionPtr
        void send(muduo::net::TcpConnection* conn,
                const muduo::StringPiece& message)
        {
            /*
               muduo::net::Buffer buf;
               buf.append(message.data(), message.size());
               int32_t len = static_cast<int32_t>(message.size());
               int32_t be32 = muduo::net::sockets::hostToNetwork32(len);
               buf.prepend(&be32, sizeof be32);
               */
            std::string msg = message.as_string();
            if(msg.find("[") == 0){
                //conn->send(encryptDecrypt(msg,mntecsecret));
                conn->send(msg);
            }else{
                std::stringstream ostr;
                ostr << "[10101,[0,\"";
                ostr << msg; 
                ostr << "\",1]]";
                std::string m;
                ostr >> m;
                //conn->send(encryptDecrypt(m,mntecsecret));
                conn->send(m);
            }
        }
       

    private:
        StringMessageCallback messageCallback_;
        const static size_t kHeaderLen = sizeof(int32_t);
};

#endif  // MUDUO_EXAMPLES_ASIO_CHAT_CODEC_H
