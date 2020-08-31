#include "ssdbluachatserver.hpp"

uint16_t port = 40010;
int numThreads = 4;

int main(int argc, char** argv)
{

    std::string argv_str(argv[0]);
    std::string cwdpath = std::string(argv_str.substr(0,argv_str.find_last_of("/")));
    MLOG_INFO << "bin dir:" << cwdpath;
    MLOG_INFO << "listening port:" << port << ", pid = " << getpid();
    long int numCpu = sysconf( _SC_NPROCESSORS_ONLN );
    numThreads = static_cast<int>(numCpu) * 2;
    if (argc > 1)
    {
        numThreads = atoi(argv[1]);
    }
    MLOG_INFO << "threads:" << numThreads << ",concurrency:" << numCpu <<    ",port:" << port;
    EventLoop loop;
    InetAddress listenAddr(port);
    //muduo::net::EventLoopThread loopThread;
    //muduo::net::EventLoop* loopInAnotherThread = loopThread.startLoop();
    CppServer server(&loop, listenAddr,numThreads);
    server.start(cwdpath);
    


    loop.loop();
}

