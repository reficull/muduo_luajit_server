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
using namespace std;
using namespace muduo;
using namespace muduo::net;

#define MONGONS "127.0.0.1:27017"
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


class EchoServer
{
private:
    TcpServer server_;
    AtomicInt64 transferred_;
    AtomicInt64 receivedMessages_;
    int64_t oldCounter_;
    Timestamp startTime_;
    typedef std::set<TcpConnectionPtr> ConnectionList;
    ConnectionList connections_;
    mongo::DBClientConnection* c;
    std::map<int,TcpConnectionPtr> userIdCon;
    
public:
    EchoServer(EventLoop* loop, const InetAddress& listenAddr)
    : server_(loop, listenAddr, "JOGServer"),
    oldCounter_(0),
    startTime_(Timestamp::now())
    {
        server_.setConnectionCallback(
                                      boost::bind(&EchoServer::onConnection, this, _1));
        server_.setMessageCallback(
                                   boost::bind(&EchoServer::onMessage, this, _1, _2, _3));
        server_.setThreadNum(numThreads);
        //loop->runEvery(10.0, boost::bind(&EchoServer::printThroughput, this));
    }
    
    void start()
    {
        MLOG_INFO << "starting " << numThreads << " threads.";
        server_.start();
        
        connMongo();
    }
    
    void reconnectMongo()
    {
        
        if (!mongo::serverAlive("127.0.0.1:27017")) {
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
                c->connect(MONGONS);
                c->auth("admin", "root", "smartintel", err);
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
            
            c->remove("Gunner.ConnectedUsers", QUERY("socket" << conn->name()));

            
           // c->remove("Gunner.Room",QUERY("p1Socket"<<conn->name()));
           // c->remove("Gunner.Room",QUERY("p2Socket"<<conn->name()));
            
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
        mongo::BSONObj b = c->findOne("Gunner.Room",QUERY("p1Socket"<<conn->name()));//作為p1
        if(b.isEmpty())
        {
            b = c->findOne("Gunner.Room",QUERY("p2Socket"<<conn->name()));//作為p2
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
    
    int getMid(string cn)
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

        c->runCommand("Gunner",bj.obj(),b);
        err = c->getLastError();
        if(!err.empty())
            return 0;
        // 获取返回内容
        mongo::BSONElement bel = b.getField("value");
        if(bel.isNull())
        {
            int tmp = 1;
            // 数据库不存在记录,创建自增ID
            c->insert("Gunner.Collection", BSON("_id"<<cn<<"currentIdValue"<<tmp));
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
            int ret = ceil(bel["currentIdValue"].Double());
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
        
        mongo::BSONObj b = c->findOne("Gunner.Room",QUERY("id" << roomNum));
        int p1Id = 0;
        p1Id = b.getIntField("p1");
        if (p1Id == 0) {
            MLOG_INFO << "db error! can not get room info";
            return;
        }
        int p2Id = b.getIntField("p2");
        updateField("Gunner.User","status",BSON("userId"<<p1Id),2);
        updateField("Gunner.User","status",BSON("userId"<<p2Id),2);
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
            updateField("Gunner.Room","win",BSON("id"<<roomNum),2);
            //c->remove("Gunner.Room",QUERY("p1Socket"<<conn->name()));
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
                        //c->remove("Gunner.Room",QUERY("p2Socket"<<it->second->name()));
                        success = true;
                    }catch(...){
                        MLOG_INFO << "mongo db failed,autoreconnect and retry after 1 sec";
                        sleep(5);
                    }
                }
                //房間狀態置1 p1勝
                updateField("Gunner.Room","win",BSON("id"<<roomNum),1);
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

       
       // std::string userId;

        std::string k,v;
        try{
            YAML::Parser parser;
            //parser.Load(sss);
            ss.seekg(0);
            parser.Load(ss);

            YAML::Node doc;
            YAML::Emitter remoteDoc;
            parser.GetNextDocument(doc);

            for(YAML::Iterator it=doc.begin();it!=doc.end();++it){
                
                it.first() >> k;
                it.second() >> v;
            }
        }
        catch(const YAML::Exception& e) {
            MLOG_INFO << "yaml failed:" << e.msg;
        }catch( mongo::ConnectException e)
        {
            MLOG_INFO << "mongodb connect failed,reconnecting";
            connMongo();
        }
        std::vector<std::string> ids;
        int d = 44;//,分隔
        split(ids,v,char(d));
        
        switch(atoi(k.c_str()))
        {
            case 20000://處理登錄
            {
                //ids[0]為 conn的userId,ids[1]為 mac
               
                std::string macAddr = ids[1];
                
                int userId = atoi(ids[0].c_str());
                doLogin(userId,conn);
                //广播给所有登录用户
                char c_userid[20];
                sprintf(c_userid,"%d",userId);
                std::string msg1 = "{protocol:20001,result:[0]}";
                conn->send(msg1);
                std::string msg = "{protocol:90000,msg:[\"" + std::string(c_userid) + " logined!\"]}";
                for (ConnectionList::iterator it = connections_.begin();
                     it != connections_.end();
                     ++it)
                {
                    if((*it)!=conn){
                        (*it)->send(msg);
                    }
                }
            }
                break;
            case 21000://註銷
            {
                //ids[0]為 userId
                int userId = atoi(ids[0].c_str());
                doLogout(userId,conn);
                
            }
                break;
                //處理挑戰
            case 10000://request fight,v 是被挑戰者userId
            {
                MLOG_INFO << "request fight,p2:" << v;
                //先檢測p2是否空閒
                mongo::BSONObj bp = doFindOne("Gunner.User",BSON("userId"<<atoi(ids[1].c_str())));
                int p2Status = bp.getIntField("status");
                if (p2Status > 2) {
                    std::string msgBusy = "{protocol:10003,result: [1]}";
                    conn->send(msgBusy);
                    return;
                }
                //find userid by conn ptr
                //ids[0]為 conn的userId,ids[1]為 p2的userId
                
                //先檢測userIdCon中是否已經有這個用戶，沒有則重連
               // reconnect(atoi(ids[0].c_str()),conn);
                //建立room,初始化數據
                mongo::BSONObj roomItem;
                
                int RoomId = getMid("Room");//獲得計算出的最大id
                mongo::BSONObj b = doFindOne("Gunner.User",BSON("userId" << atoi(ids[0].c_str())));
               
                std::string nickName = b.getStringField("nickname");
                int image = 0;
                image = b.getIntField("image");
                if (image == 0) {
                    image = 1;
                }
                
                /* 返回
                 {10002: [0,roomNum,enemyHP,enemyUserId]} 發給conn
                 {10001: [0,roomNum,enemyHP,enemyUserId,enemyName,enemyImage]} 發給對手
                 */
                char msg2[200] = {0};
                char msg1[200] = {0};
                sprintf(msg2,"{protocol:10001,result: [0,%d,%d,%s,\"%s\",%d]}",RoomId,100,ids[0].c_str(),nickName.c_str(),image);
                std::string msg2Str = std::string(msg2);
                sprintf(msg1,"{protocol:10002,result: [0,%d,%d,%s]}",RoomId,100,ids[1].c_str());
                std::string msg = std::string(msg1);
                
                
                //TODO: 如果p2忙，則做相應 處理
                std::string p2socketName;
                std::map<int,TcpConnectionPtr>::iterator it = userIdCon.find(atoi(ids[1].c_str()));
                if(it != userIdCon.end())
                {
                    MLOG_INFO << "send startfight to p2:" << msg2Str;
                    MLOG_INFO << "send confirm fight to p1:" << msg;
                    p2socketName = it->second->name();
                    it->second->send(msg2Str);//發給p2
                    conn->send(msg);//發給p1
                    roomItem = BSON("id" << RoomId << "startTime" << mongo::DATENOW <<  "p1" << atoi(ids[0].c_str()) << "p1HP" << 100  << "p1Socket" << conn->name() << "p2" << atoi(ids[1].c_str()) << "p2HP" << 100 << "p2Socket" << p2socketName << "win" << 0);
                    bool success = false;
                    
                    while (!success) {
                        try{
                            c->update("Gunner.Room",QUERY("p1" << atoi(ids[0].c_str())),roomItem,1);
                            incField("User",BSON("userId"<<atoi(ids[0].c_str())),"status");
                            incField("User",BSON("userId"<<atoi(ids[1].c_str())),"status");
                            success = true;
                        }catch(...){
                            MLOG_INFO << "mongo db failed,autoreconnect and retry after 1 sec";
                            sleep(3);
                        }
                    }
                }else{
                    //找不到p2在線信息
                    //只發給p1 測試用，正式服會刪除，或者 增加機器人應戰
                    conn->send(msg);//發給p1
                    roomItem = BSON("id" << RoomId << "startTime" << mongo::DATENOW <<  "p1" << atoi(ids[0].c_str()) << "p1HP" << 100  << "p1Socket" << conn->name() << "p2" << atoi(ids[1].c_str()) << "p2HP" << 100 << "p2Socket" << p2socketName << "win" << 0);
                    bool success = false;
                    
                    while (!success) {
                        try{
                            c->update("Gunner.Room",QUERY("p1" << atoi(ids[0].c_str())),roomItem,1);//QUERY("p1" << atoi(ids[0].c_str()))
                            incField("User",BSON("userId"<<atoi(ids[0].c_str())),"status");
                            success = true;
                        }catch(...){
                            MLOG_INFO << "mongo db failed,autoreconnect and retry after 1 sec";
                            sleep(3);
                        }
                    }
                    //生產服應該發失敗信息，可能是對手忙碌中，或不在線
                    //{"protocol":10001, "result":[1,string]} 失敗 原因

                }
            }
                break;
            case 10010:
            case 10005:
            {
                /*
                 打中
                 {10010 : [decreaseHP,roomNum,selfUserId,seqNum,type]}
                 打死下行
                 {"protocol":10016,"result":[1/2]} 2是輸，1是贏
                 
                 本人擊發上行
                 ｛10005:[decreaseHP,roomNum,selfUserId,seqNum]}
                 擊發下行
                 {protocol:10006,result:[0]}
                 */
                std::string msgWin = "{protocol:10016,result:[1]}";
                std::string msgLoose = "{protocol:10016,result:[2]}";
                
                //ids[0]為 conn的userId,ids[1]為 p2的userId
                MLOG_INFO << "hit enemy decrease:" << ids[0] << " roomNum:" << ids[1] << " attackerid:" << ids[2] << " seqNum:" << ids[3];
                bool success = false;
                mongo::BSONObj b;
                while (!success) {
                    try{
                        b = c->findOne("Gunner.Room",QUERY("id" << atoi(ids[1].c_str())));
                        success = true;
                    }catch(...){
                        MLOG_INFO << "mongo db failed,autoreconnect and retry after 1 sec";
                        sleep(5);
                    }
                }
                
                std::string msgBeAttacking = "{protocol:10006,result:[0]}";
                
                char msgCstrAttack[200] = {0};
                char msgCstrBeHit[200] = {0};
                int p1Id = 0;
                p1Id = b.getIntField("p1");
                if (p1Id == 0) {
                    MLOG_INFO << "db error! can not get room info";
                    return;
                }
                int p2Id = b.getIntField("p2");
                if(p1Id == atoi(ids[2].c_str())){
                    int win = b.getIntField("win");
                    if (win==1) {
                        conn->send(msgWin);
                        return;
                    }else if(win ==2 ){
                        conn->send(msgLoose);
                        return;
                    }
                    if(k.compare("10010")==0){
                        //p1是攻擊 者，減p2的hp 發給p1:{"protocol":10011,"result":[damageNum,type]}確認打中
                        //發給p2:{"protocol":10012,"result":[damageNum,type]}被打中，扣自己的hp
                        int p2HP = b.getIntField("p2HP");
                        p2HP = p2HP - atoi(ids[0].c_str());// 計算剩下 多少
                        if (p2HP > 0) {
                            char p2HPStr[4];
                            sprintf(p2HPStr,"%d",p2HP);
                            
                            sprintf(msgCstrAttack,"{protocol:10011,result:[%s,%s]}",p2HPStr,ids[4].c_str());
                            sprintf(msgCstrBeHit,"{protocol:10012,result:[%s,%s]}",p2HPStr,ids[4].c_str());
                            
                            
                            std::map<int,TcpConnectionPtr>::iterator it = userIdCon.find(p2Id);
                            if(it != userIdCon.end()){//說明和沒有連接網絡的死人打
                                MLOG_INFO << "send to be hit msg:" << msgCstrBeHit;
                                it->second->send(msgCstrBeHit);
                            }else{
                                MLOG_INFO << "和不在線的人打，不發擊中信息";
                            }
                            MLOG_INFO << "send to attacker msg:" << msgCstrAttack;
                            TcpConnectionPtr p1Conn;
                            while(!p1Conn)
                            {
                                p1Conn = userIdCon[atoi(ids[2].c_str())];
                                if(!p1Conn)
                                {
                                    doLogin(atoi(ids[2].c_str()),conn);
                                }
                            }
                            p1Conn->send(msgCstrAttack);
                            mongo::BSONObjBuilder bj;
                            mongo::BSONObj newRoom = b.removeField("p2HP");
                            
                            bj.appendElements(mongo::fromjson(newRoom.jsonString()));
                            bj.append("p2HP",p2HP);
                            success = false;
                            
                            while (!success) {
                                try{
                                    c->update("Gunner.Room",QUERY("id" << atoi(ids[1].c_str())),bj.obj(),true);
                                    success = true;
                                }catch(...){
                                    MLOG_INFO << "mongo db failed,autoreconnect and retry after 1 sec";
                                    sleep(5);
                                }
                            }
                        }else{
                            //通知 p1勝利 ,p2輸
                            std::map<int,TcpConnectionPtr>::iterator it = userIdCon.find(p2Id);
                            if (it != userIdCon.end()) {
                                MLOG_INFO << "send loose msg:" << msgLoose << " to:" << p2Id;
                                it->second->send(msgLoose);
                            }
                            
                            MLOG_INFO << "send win msg:" << msgWin << " to:" << p1Id;
                            conn->send(msgWin);
                            updateField("Gunner.Room","win",BSON("id"<<atoi(ids[1].c_str())),1);
                            incField("User",BSON("userId"<<p1Id),"win");
                            incField("User",BSON("userId"<<p2Id),"lose");
                            updateField("Gunner.User","status",BSON("userId"<<p1Id),2);
                            updateField("Gunner.User","status",BSON("userId"<<p2Id),2);
                        }
                    }else if(k.compare("10005")==0){//發出攻擊 ，讓對方播放攻擊效果
                        std::map<int,TcpConnectionPtr>::iterator it = userIdCon.find(p2Id);
                        if(it != userIdCon.end()){//說明和沒有連接網絡的死人打
                            MLOG_INFO << "send to be attacked msg:" << msgBeAttacking;
                            it->second->send(msgBeAttacking);
                        }else{
                            MLOG_INFO << "和不在線的人打，不發打擊信息";
                        }
                    }
                }else if(p2Id == atoi(ids[2].c_str())){
                    //p2是攻擊者，減p1的hp
                    //發給p1 {"protocol":10012,"result":[damageNum]}被打中，扣自己的hp
                    //發給p2 {"protocol":10011,"result":[damageNum]}確認打中
                    int win = b.getIntField("win");
                    if (win==1) {
                        conn->send(msgLoose);
                        return;
                    }else if(win ==2 ){
                        conn->send(msgWin);
                        return;
                    }
                    if(k.compare("10010")==0){
                        int p1HP =  b.getIntField("p1HP");
                        p1HP = p1HP - atoi(ids[0].c_str());// 計算剩下 多少
                        if (p1HP > 0) {
                            char p1HPStr[4];
                            sprintf(p1HPStr,"%d",p1HP);
                            sprintf(msgCstrAttack,"{protocol:10011,result:[%s,%s]}",p1HPStr,ids[4].c_str());
                            sprintf(msgCstrBeHit,"{protocol:10012,result:[%s,%s]}",p1HPStr,ids[4].c_str());
                            
                            MLOG_INFO << "send to attacker msg:" << msgCstrAttack;
                            conn->send(msgCstrAttack); //conn是p2,確認擊中
                            
                            std::map<int,TcpConnectionPtr>::iterator it = userIdCon.find(p1Id);
                            if(it != userIdCon.end()){
                                MLOG_INFO << "send to be hit msg:" << msgCstrBeHit;
                                it->second->send(msgCstrBeHit);//被擊中
                            }else{
                                MLOG_INFO << "fatal error, 在線人員中找不到請求者";
                            }
                            mongo::BSONObjBuilder bj;
                            mongo::BSONObj newRoom = b.removeField("p1HP");
                            
                            bj.appendElements(mongo::fromjson(newRoom.jsonString()));
                            bj.append("p1HP",p1HP);
                            
                            
                            success = false;
                            
                            while (!success) {
                                try{
                                    c->update("Gunner.Room",QUERY("id" << atoi(ids[1].c_str())),bj.obj(),true);
                                    success = true;
                                }catch(...){
                                    MLOG_INFO << "mongo db failed,autoreconnect and retry after 5 sec";
                                    sleep(5);
                                }
                            }
                        }else{
                            //通知p2勝，p1輸
                            std::map<int,TcpConnectionPtr>::iterator it = userIdCon.find(p1Id);
                            MLOG_INFO << "send loose msg:" << msgLoose << " to:" << p1Id;
                            it->second->send(msgLoose);
                            MLOG_INFO << "send win msg:" << msgWin << " to:" << p2Id;
                            conn->send(msgWin);
                            updateField("Gunner.Room","win",BSON("id"<<atoi(ids[1].c_str())),2);
                            incField("User",BSON("userId"<<p1Id),"lose");
                            incField("User",BSON("userId"<<p2Id),"win");
                            updateField("Gunner.User","status",BSON("userId"<<p1Id),2);
                            updateField("Gunner.User","status",BSON("userId"<<p2Id),2);
                        }
                    }else if(k.compare("10005")==0){
                        std::map<int,TcpConnectionPtr>::iterator it = userIdCon.find(p1Id);
                        if(it != userIdCon.end()){
                            MLOG_INFO << "send to be attacking msg:" << msgBeAttacking;
                            it->second->send(msgBeAttacking);//對方攻擊中
                        }else{
                            MLOG_INFO << "fatal error, 在線人員中找不到請求者";
                        }
                    }
                }

            }
                break;
            case 10020:
            {
                //處理逃跑
                
                MLOG_INFO << "escape msg,roomNum:" << ids[0] << " player id:" << ids[1];
                escape(atoi(ids[0].c_str()),atoi(ids[1].c_str()),conn);
                incField("User",BSON("userId"<<atoi(ids[1].c_str())),"escape");
                
            }
            default:
                ;
        }
        
        
        
        //處理登錄
        /*
        if(act.compare("getPlayerList") == 0)
        {
            mongo::Query q = QUERY("status" << 0);
            
            std::vector<mongo::BSONObj> out;
            c->findN(out, "Gunner.ConnectedUsers", q, 10);
            for (std::vector<mongo::BSONObj>::iterator it; it!=out.end(); ++it) {
                mongo::BSONObj b = *it;
                MLOG_INFO << "user detail:" << b.toString();
            }
             
            std::string ret = "{{type:playerList},list:{";
            std::auto_ptr<mongo::DBClientCursor> cursor = c->query("Gunner.ConnectedUsers",QUERY("status" << 0));
            while(cursor->more())
            {
                mongo::BSONObj p = cursor->next();
               
                //ret += "{";
                ret += p.toString();
                ret += ",";
            }
            
            ret = ret.substr(0,ret.size()-1);
            ret += "}}";
             std::cout << ret << std::endl;
            conn->send(ret);
        }
        */
        //printThroughput();
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
                c->update("Gunner.ConnectedUsers",QUERY("userId"<<userId), statusItem,true);
                MLOG_INFO << "connectedUsers added:" << userId;
                success = true;
            }catch(...){
                MLOG_INFO << "mongo db failed,autoreconnect and retry after 5 sec";
                sleep(5);
            }
        }
        
        c->insert("Gunner.LogConnect",statusItem);
        updateField("Gunner.User", "status", BSON("userId" << userId), ONLINESTATUS);

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
        c->runCommand("Gunner",bj.obj(),b);
        string err;
        err = c->getLastError();
        if(!err.empty()){
            MLOG_INFO << "inc field error:" << err;
            updateField("Gunner.User", field, q, 1);
            
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
        updateField("Gunner.User", "status", BSON("userId" << userId), OFFLINESTATUS);

        informRival(userId,conn);
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

int main(int argc, char* argv[])
{
    MLOG_INFO << "pid = " << getpid() << ", tid = " << CurrentThread::tid();
    if (argc > 1)
    {
        numThreads = atoi(argv[1]);
    }
    EventLoop loop;
    InetAddress listenAddr(9999);
    EchoServer server(&loop, listenAddr);
    
    server.start();
    
    loop.loop();
}

