#include "muduo/net/TcpConnection.h"
#include <vector>

typedef std::vector<muduo::net::TcpConnectionPtr> tcons;

class map{
    public:
        map();
        ~map();

        int loadMap();
        tcons getUser(uint16_t x1,uint16_t x2,uint16_t y1,uint16_t y2);

};
