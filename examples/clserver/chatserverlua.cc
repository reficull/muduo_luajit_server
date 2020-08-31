#include <luajit-2.1/lua.hpp>
#include <fcntl.h>
#include <sys/stat.h> 
#include <sys/types.h> 
//#include <ev.h>
//#include <inotify.h>
//#include <oolua/oolua.h>
#include <muduo/base/Atomic.h>
#include <muduo/base/Logging.h>
#include <muduo/base/Thread.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/InetAddress.h>
#include <muduo/net/TcpServer.h>
#include <boost/bind.hpp>
#include <set>
#include <map>
#include <vector>
#include <utility>
#include <stdio.h>
#include <unistd.h>
#include <sstream>
#include <iostream>
#include <pcre++.h>
#include <yaml-cpp/yaml.h>
#include <mongo/client/dbclient.h>
#include <time.h>
#include <memory>
#include <libconfig.h++>

using namespace std;
using namespace muduo;
using namespace muduo::net;
using namespace libconfig;
#define CONFIGFILE "chatserverconfig.cfg"
#define ONLINESTATUS 2
#define OFFLINESTATUS 1

#define MLOG_INFO if (muduo::Logger::logLevel() <= muduo::Logger::INFO) \
                                                   muduo::Logger(__FILE__, __LINE__).stream()

int numThreads = 4;
void split(vector<string> &tokens,  string &text, char sep) {
    unsigned long start = 0, end = 0;
    while ((end = text.find(sep, start)) != string::npos) {
        tokens.push_back(text.substr(start, end - start));
        start = end + 1;
    }
    tokens.push_back(text.substr(start));
}

template <typename T>
void getConfig(const char *key,T &v)
{
    Config cfg;
    try
    {
        cfg.readFile(CONFIGFILE);
    }
    catch(const FileIOException &fioex)
    {
        std::cerr << "I/O error while reading file." << std::endl;
        cout << EXIT_FAILURE << endl;
    }
    catch(const ParseException &pex)
    {
        std::cerr << "Parse error at " << pex.getFile() << ":" << pex.getLine()
            << " - " << pex.getError() << std::endl;
        cout << EXIT_FAILURE << endl;
    }

    try
    {
        cfg.lookupValue(key,v);
        cout << key << ": " << v << endl << endl;
    }
    catch(const SettingNotFoundException &nfex)
    {
        //cerr << "No 'mongons' setting in configuration file." << endl;
        exit(-1);
    }
}
template void getConfig(const char*key,int&);
template void getConfig(const char*key,std::string&);
/*
   std::string currentDateTime() {
   time_t     now = time(0);
   struct tm  tstruct;
   char       buf[80];
   tstruct = *localtime(&now);
// Visit http://www.cplusplus.com/reference/clibrary/ctime/strftime/
// for more information about date/time format
strftime(buf, sizeof(buf), "%Y-%m-%d %X", &tstruct);

return buf;
}
*/
class Lua_State
{
    lua_State *L;
    public:
    Lua_State() : L(luaL_newstate()) {
        // load Lua libraries
        luaL_openlibs(L);
    }

    ~Lua_State() {
        lua_close(L);
    }

    // implicitly act as a lua_State pointer
    inline operator lua_State*() {
        return L;
    }
};

class EchoServer
{
    private:
        Lua_State L;
        TcpServer server_;
        std::string mongons;
        AtomicInt64 transferred_;
        AtomicInt64 receivedMessages_;
        int64_t oldCounter_;
        Timestamp startTime_;
        typedef std::set<TcpConnectionPtr> ConnectionList;
        ConnectionList connections_;
        mongo::DBClientConnection* c;
        std::map<int,TcpConnectionPtr> userIdCon;
        std::string mongouser;
        std::string mongopasswd;
        Config cfg;
        std::string accessTime;

    public:
        EchoServer(EventLoop* loop, const InetAddress& listenAddr)
            : server_(loop, listenAddr, "chatServer"),
            oldCounter_(0),
            startTime_(Timestamp::now())
    {
        server_.setConnectionCallback(
                boost::bind(&EchoServer::onConnection, this, _1));
        server_.setMessageCallback(
                boost::bind(&EchoServer::onMessage, this, _1, _2, _3));
        server_.setThreadNum(numThreads);
        loop->runEvery(10.0, boost::bind(&EchoServer::printThroughput, this));
        loop->runEvery(10.0, boost::bind(&EchoServer::checkRenew, this));
    }
        void loadLua()
        {

            // run the Lua script
            luaL_dofile(L, "scripts/main.lua");
        }
        void start()
        {
            MLOG_INFO << "starting " << numThreads << " threads.";
            server_.start();

            getConfig("mongons",mongons);
            getConfig("mongouser",mongouser);
            getConfig("mongopasswd",mongopasswd);
            accessTime = std::string("");
            checkRenew();
            connMongo();
        }

        void reconnectMongo()
        {
            if (!mongo::serverAlive(mongons)) {
                connMongo();
            }
        }

        void connMongo()
        {
            std::string err;
            bool conSuccess = false;
            while(!conSuccess){
                try{
                    // c.autoReconnect = 1;
                    c = new mongo::DBClientConnection(true,0,0);
                    c->connect(mongons);
                    c->auth("admin", mongouser, mongopasswd, err);
                    MLOG_INFO << "mongo login " << err;
                    conSuccess = true;
                }
                catch( mongo::ConnectException e)
                {
                    MLOG_INFO << "mongodb connect failed wait 5 secs";
                    sleep(5);
                }
            }
        }
    private:
        void onConnection(const TcpConnectionPtr& conn)
        {
            MLOG_INFO << conn->peerAddress().toIpPort() << " -> "
                << conn->localAddress().toIpPort() << " is "
                << (conn->connected() ? "UP" : "DOWN");
            conn->setTcpNoDelay(true);
            if (conn->connected())
            {
                connections_.insert(conn);
            }
            else
            {

                c->remove("gameInfo.ConnectedUsers", QUERY("socket" << conn->name()));


                // c->remove("gameInfo.Room",QUERY("p1Socket"<<conn->name()));
                // c->remove("gameInfo.Room",QUERY("p2Socket"<<conn->name()));

                for(std::map<int,TcpConnectionPtr>::iterator it = userIdCon.begin(); it != userIdCon.end();++it){
                    if(conn == it->second)
                    {

                        doLogout(it->first,conn);//通知對手，更新庫等等
                        userIdCon.erase(it);
                        MLOG_INFO << "user id:" << it->first << " Logout!";
                        break;
                    }
                }
                connections_.erase(conn);
                //MLOG_INFO << "connected:" << connections_.size() << "," << userIdCon.size();
            }
        }

        void informRival(int userId,const TcpConnectionPtr& conn)
        {
            mongo::BSONObj b = c->findOne("gameInfo.Room",QUERY("p1Socket"<<conn->name()));//作為p1
            if(b.isEmpty())
            {
                b = c->findOne("gameInfo.Room",QUERY("p2Socket"<<conn->name()));//作為p2
            }
            if(!b.isEmpty())
            {
                int roomId = b.getIntField("id");
                escape(roomId,userId,conn);
            }
        }

        void reconnect(int userId,const TcpConnectionPtr& conn)
        {
            bool found = 0;
            for(std::map<int,TcpConnectionPtr>::iterator it = userIdCon.begin(); it != userIdCon.end();++it){
                if(conn == it->second)
                {
                    found = 1;
                }
            }
            if(!found)
            {
                MLOG_INFO << "user reconnected:" << userId;
                doLogin(userId,conn);
            }
        }

        double getMid(string cn)
        {
            string err;
            mongo::BSONObj b;

            mongo::BSONObjBuilder bj;
            //　当前集合Collection
            bj.append("findAndModify","Collection");
            //　查询串
            bj.append("query",BSON("_id"<<cn));
            // 更新串
            bj.append("update",BSON("$inc"<<BSON("currentIdValue"<<1)));

            c->runCommand("gameInfo",bj.obj(),b);
            err = c->getLastError();
            if(!err.empty())
                return 0;
            // 获取返回内容
            mongo::BSONElement bel = b.getField("value");
            if(bel.isNull())
            {
                int tmp = 1;
                // 数据库不存在记录,创建自增ID
                c->insert("gameInfo.Collection", BSON("_id"<<cn<<"currentIdValue"<<tmp));
                err = c->getLastError();
                if(err.empty())
                {
                    return 1;
                }
                return 0;
            }
            // 当EL是结束
            if(bel.eoo()) return 0;

            // 检查类型是INT或DOUBLE
            if(bel.type() == mongo::NumberDouble)
            {
                double ret = ceil(bel["currentIdValue"].Double());
                return ret;
            }else{
                return bel["currentIdValue"].Int();
            }
        }

        void escape(int roomNum,int userId,const TcpConnectionPtr& conn)
        {
            /*
               逃跑 {10020:[roomNum, selfUserId]}
               下行 {"protocol":10021,"result":0/1} 0是自己，1是对方 由服务器判断
               */
            std::string msgConfirm = "{protocol:10021,result:[0]}";
            std::string msgInform = "{protocol:10021,result:[1]}";

            mongo::BSONObj b = c->findOne("gameInfo.Room",QUERY("id" << roomNum));
            int p1Id = 0;
            p1Id = b.getIntField("p1");
            if (p1Id == 0) {
                MLOG_INFO << "db error! can not get room info";
                return;
            }
            int p2Id = b.getIntField("p2");
            updateField("gameInfo.User","status",BSON("userId"<<p1Id),2);
            updateField("gameInfo.User","status",BSON("userId"<<p2Id),2);
            if(p1Id == userId){
                //P1逃跑，發確認信息給P1,通知P2
                conn->send(msgConfirm);
                std::map<int,TcpConnectionPtr>::iterator it = userIdCon.find(p2Id);
                if(it != userIdCon.end()){//說明和沒有連接網絡的死人打
                    MLOG_INFO << "send escape msg:" << msgInform << "to :" << p2Id;
                    it->second->send(msgInform);
                }else{
                    MLOG_INFO << "和不在線的人打，不發逃跑信息";
                }
                //房間狀態置2 p2勝
                updateField("gameInfo.Room","win",BSON("id"<<roomNum),2);
                //c->remove("gameInfo.Room",QUERY("p1Socket"<<conn->name()));
            }else if (p2Id == userId){
                //P2逃跑，發確認信息給P2，通知P1
                conn->send(msgConfirm);
                std::map<int,TcpConnectionPtr>::iterator it = userIdCon.find(p1Id);
                if(it != userIdCon.end()){//說明和沒有連接網絡的死人打
                    MLOG_INFO << "send escape msg:" << msgInform << "to :" << p1Id;
                    it->second->send(msgInform);

                    bool success = false;

                    while (!success) {
                        try{
                            //c->remove("gameInfo.Room",QUERY("p2Socket"<<it->second->name()));
                            success = true;
                        }catch(...){
                            MLOG_INFO << "mongo db failed,autoreconnect and retry after 1 sec";
                            sleep(5);
                        }
                    }
                    //房間狀態置1 p1勝
                    updateField("gameInfo.Room","win",BSON("id"<<roomNum),1);
                }else{
                    MLOG_INFO << "和不在線的人打，不發逃跑信息";
                }

            }
        }



        void onMessage(const TcpConnectionPtr& conn, Buffer* buf, Timestamp)
        {
            size_t len = buf->readableBytes();
            transferred_.addAndGet(len);
            receivedMessages_.incrementAndGet();
            //MLOG_INFO << "received:" << buf->retrieveAllAsString();
            std::string rc(buf->retrieveAllAsString());
            // muduo::StringPiece sp = muduo::StringPiece(rc);
            MLOG_INFO << "get muduo string:" <<  rc;
            // const char* cstr = rc.c_str();
            //std::string sstr = cstr;


            //pcrepp::Pcre pp = pcrepp::Pcre("\"",PCRE_GLOBAL);
            //sstr = pp.replace(sstr,"\\\"");
            std::stringstream ss(rc);
            for (ConnectionList::iterator it = connections_.begin();
                    it != connections_.end();
                    ++it)
            {
                if((*it)!=conn){
                    (*it)->send(rc);
                }
            }

}

void doLogin(int userId,const TcpConnectionPtr& conn)
{
    long long seconds;
    seconds = time(NULL);
    //写mongodb
    //mongo::BSONObjBuilder statusBuilder;
    mongo::BSONObj statusItem;
    //status 0登录空闲 1busy
    statusItem = BSON("serverid" << 1 << "type" << 10001 << "socket" << conn->name()  << "userId" << userId << "loginTime" << mongo::DATENOW << "loginTimeSecs" << seconds << "status" << 0 << "updateTime" << mongo::DATENOW);
    bool success = false;

    while (!success) {
        try{
            c->update("gameInfo.ConnectedUsers",QUERY("userId"<<userId), statusItem,true);
            MLOG_INFO << "connectedUsers added:" << userId;
            success = true;
        }catch(...){
            MLOG_INFO << "mongo db failed,autoreconnect and retry after 5 sec";
            sleep(5);
        }
    }

    c->insert("gameInfo.LogConnect",statusItem);
    updateField("gameInfo.User", "status", BSON("userId" << userId), ONLINESTATUS);

    //在onCOnnect時已經將之前的記錄刪除
    std::map<int,TcpConnectionPtr>::iterator it = userIdCon.find(userId);
    if(it == userIdCon.end()){
        userIdCon.insert(make_pair(userId,conn));
    }
}

mongo::BSONObj doFindOne(std::string ns,mongo::BSONObj q){
    mongo::BSONObj b;
    bool success = false;
    while (!success) {
        try{
            b = c->findOne(ns,q);
            success = true;
        }catch(...){
            MLOG_INFO << "mongo db failed,autoreconnect and retry after 1 sec";
            sleep(5);
        }
    }
    return b;
}

int incField(std::string collection,mongo::BSONObj q,std::string field)
{
    mongo::BSONObjBuilder bj;
    //　当前集合Collection
    bj.append("findAndModify",collection);
    //　查询串
    bj.append("query",q);//BSON("_id"<<cn)
    // 更新串
    bj.append("update",BSON("$inc"<<BSON(field<<1)));
    mongo::BSONObj b;
    c->runCommand("gameInfo",bj.obj(),b);
    string err;
    err = c->getLastError();
    if(!err.empty()){
        MLOG_INFO << "inc field error:" << err;
        updateField("gameInfo.User", field, q, 1);

        return 0;
    }
    return 1;
}


    template <class T>
void updateField(std::string ns,std::string field,mongo::BSONObj filter,T a)
{
    mongo::BSONObj u = c->findOne(ns,filter);

    mongo::BSONObjBuilder bj;

    u = u.removeField(field);

    mongo::BSONObj tmp = u.copy();
    // MLOG_INFO << "get user info:" <<  i_userId << ":" << tmp.jsonString();
    try{
        bj.appendElements(mongo::fromjson(u.jsonString()));
        bj.append(field , a);

        c->update(ns,filter,bj.obj());
        //MLOG_INFO << "update " << ns ":" << field;
    }
    catch( ...)
    {
        MLOG_INFO << "mongodb query failed  " << filter.jsonString();
    }
}

void doLogout(int userId,const TcpConnectionPtr& conn){
    updateField("gameInfo.User", "status", BSON("userId" << userId), OFFLINESTATUS);

    informRival(userId,conn);
}
void checkRenew()
{
    int f_d = 0; 
    struct stat st;
    f_d = open("scripts/renew.lua",O_RDONLY); 
    if(-1 == f_d) 
    { 
        cout << "\n NULL File descriptor\n"; 
        return;
    } 
    errno = 0; 
    if(fstat(f_d, &st)) 
    { 
        cout << "\nfstat error: [%s]\n" << strerror(errno) << endl; 
        close(f_d); 
        return; 
    } 
    if(accessTime != ctime(&st.st_atime)){
        cout << "file touched:" << ctime(&st.st_atime) << endl;
        accessTime = ctime(&st.st_atime);
        loadLua();
    }
    close(f_d);
    /*
    if(!fd){
        fd = FileTools::addWatch("scripts/renew.lua");
        cout << "add watch renew.lua" << endl;
    }
    if(FileTools::hasChanged(fd)){
        cout << "file renewed,reloading!" << endl;
    }
    */
}
void printThroughput()
{
    /*
       Timestamp endTime = Timestamp::now();
       int64_t newCounter = transferred_.get();
       int64_t bytes = newCounter - oldCounter_;
       int64_t msgs = receivedMessages_.getAndSet(0);
       double time = timeDifference(endTime, startTime_);
       printf("%4.3f MiB/s %4.3f Ki Msgs/s %6.2f bytes per msg\n",
       static_cast<double>(bytes)/time/1024/1024,
       static_cast<double>(msgs)/time/1024,
       static_cast<double>(bytes)/static_cast<double>(msgs));
       oldCounter_ = newCounter;
       startTime_ = endTime;

       for (ConnectionList::iterator it = connections_.begin();
       it != connections_.end();
       ++it)
       {
       (*it)->send("{protocol:00001}");
       }
       */
    for(std::map<int,TcpConnectionPtr>::iterator it = userIdCon.begin(); it != userIdCon.end();++it){
        it->second->send("{protocol:00001}");
    }
}
};
/*
void initWatch()
{
    fd_inotify_ = inotify_init1(IN_NONBLOCK);
    if( fd_inotify_ < 0 ) 
    {   
        return INOT_INIT_ERR;
    }   

}
*/
int main(int argc, char* argv[])
{
    MLOG_INFO << "pid = " << getpid() << ", tid = " << CurrentThread::tid();
    if (argc > 1)
    {
        numThreads = atoi(argv[1]);
    }
    int port;
    getConfig("port",port);
    EventLoop loop;
    InetAddress listenAddr(40010);
    EchoServer server(&loop, listenAddr);

    server.start();
    


    loop.loop();
}

