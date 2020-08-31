#include "ssdbluachatserver.hpp"
//c++ connection object send to lua
#include "tcon.hpp"
//c++ object send to lua
#include "C2lua.hpp"
//timestamp send to lua
#include "tstamp.hpp"


const char* ssdbIP;
int ssdbPort;
std::string cwdpath;
char mntecsecret[10] = {0};
typedef std::tuple<int64_t,int64_t,std::string> tuple_msg;
/*
static bool
str_to_uint16(const char *str, uint16_t *res)
{
    char *end;
    errno = 0;
    intmax_t val = strtoimax(str, &end, 10);
    if (errno == ERANGE || val < 0 || val > UINT16_MAX || end == str || *end != '\0')
        return false;
    *res = (uint16_t) val;
    return true;
}
*/


void PrintTable(lua_State *L)
{
    lua_pushnil(L);

    while(lua_next(L, -2) != 0)
    {
        if(lua_isstring(L, -1)){
            MLOG_INFO <<  lua_tostring(L, -2) << " = " <<lua_tostring(L, -1);
        }else if(lua_isnumber(L, -1)){
            MLOG_INFO <<  lua_tostring(L, -2) << " = " <<  lua_tonumber(L, -1);
        }else if(lua_istable(L, -1)){
            PrintTable(L);
        }

        lua_pop(L, 1);
    }
}
void split(vector<string> &tokens,  string &text, char sep) {
    unsigned long start = 0, end = 0;
    while ((end = text.find(sep, start)) != string::npos) {
        tokens.push_back(text.substr(start, end - start));
        start = end + 1;
    }
    tokens.push_back(text.substr(start));
}        
void printMessage(const std::string& s) {
        MLOG_INFO << s;
}
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

struct stat getAccessTime()
{
    int f_d = 0;
    struct stat st;
    f_d = open("scripts/renew.lua",O_RDONLY);
    if(-1 == f_d)
    {
        MLOG_INFO << "\n NULL File descriptor\n";
        return st;
    }
    errno = 0;
    if(fstat(f_d, &st))
    {
        MLOG_INFO << "\nfstat error: [%s]\n" << strerror(errno) ;
        close(f_d);
        return st;
    }
    close(f_d);
    return st;
}

void CppServer::handleLogout(const TcpConnectionPtr* conn,std::string& reason){
    MLOG_INFO << "ssdbluachat handle logout called";
    // do not care about real connection, program logout not equal real socket disconnect, let c++ engine handle the real disconnection
    if(!*conn){
        MLOG_INFO << "conn not exists, return!" ;
        return;
    }
    cleanup(*conn);
    if(*conn){
        //(*conn)->send("disconnecting:"+reason);
        (*conn)->forceClose();
    }
    return;
}
bool CppServer::isWsClient(const TcpConnectionPtr* conn){
    if(msgUList[(*conn)] == MT_WEBSOCKET){
        return true;
    }else{
        return false;
    }
}
void CppServer::lsendMsg(const TcpConnectionPtr* conn,std::string& msg){
//    MLOG_INFO << "ssdbluachat csendMsg called:" << msg ;
    //std::stringstream buffer;
    //msgpack::pack(buffer,msg);
    //std::string packedstr = buffer.str();
    MLOG_INFO << "check msg type:" << msgUList[(*conn)];
    if(msgUList[(*conn)] == MT_WEBSOCKET){
        MLOG_INFO << "this is websocket client" ;
        unsigned char* buffer=new unsigned char[1024];
        unsigned char* msgws = reinterpret_cast<unsigned char*>(const_cast<char*>(msg.c_str()));
        ws.makeFrame(TEXT_FRAME,msgws,static_cast<int>(msg.length()),buffer,0);
        msg = std::string(reinterpret_cast<const char*>(buffer)); 
        MLOG_INFO << "msg in ws frame:" << msg;
        //rewrite this with RAII pattern
        delete buffer;
    }
    //MLOG_INFO << "gonna send in lsendMsg::" << msg; 
    if(*conn){ 
 //       MLOG_INFO << "gonna send in lsendMsg not ws client:" << msg; 
        try{
            (*conn)->send(msg);
            MLOG_INFO << __FILE__ << ":" << __LINE__ <<  ":lsendmsg sent! ";
        }catch(const char* em){
            MLOG_INFO << "error msg:" << em;
        }
    }else{
        MLOG_INFO << "connection not exists! ";
    }
}

void CppServer::loadConfig()
{
    L = luaL_newstate();
    MLOG_INFO << " lua new state......." ;
    luaL_openlibs(L);
    MLOG_INFO << " lua open libs ......." ;
    std::string spath = cwdpath + "/scripts/config.lua";
    if(luaL_loadfile(L, spath.c_str()) || lua_pcall(L,0,0,0)){
        MLOG_INFO << "config not loaded,quit immedately";    
        exit(0);
    }
    MLOG_INFO << " load config: " << spath <<  "......." ;

    ssdbPort = lua_intexpr(L, "config.ssdbPort");
    ssdbIP = lua_stringexpr(L, "config.ssdbIP");
    MLOG_INFO << " ssdb config: " << ssdbIP << ":" <<  ssdbPort <<  "......." ;

    //mapFileName = std::string(lua_stringexpr(L,"SERVER.mapFile"));
    //strncpy(mntecsecret,config["SERVER"]["secret"].cast<std::string>().c_str(),sizeof(mntecsecret)-1); 
    //strncpy(mntecsecret,lua_stringexpr(L,"SERVER.secret"),sizeof(mntecsecret)-1); 
    MLOG_INFO << "ssdb port:" << ssdbPort << ",ssdb ip:" << ssdbIP;// << ",secret:" << mntecsecret;
}

void CppServer::loadLua(int firstTime)
{
    //getGlobalNamespace(L).beginNamespace("DB").addCFunction("hset",&CppServer::hset).endNamespace();

    // run the Lua script
    if(firstTime){
        std::string spath = cwdpath + "/scripts/main.lua";
        Lunar<TCON>::Register(L);
        Lunar<TSTAMP>::Register(L);
        Lunar<C2lua>::Register(L);
        MLOG_INFO << "cwd:" << cwdpath ;
        if(luaL_dofile(L, spath.c_str())){
            MLOG_INFO << "ERROR:" << lua_tostring(L,-1) <<  ",main.lua not loaded,check the code";    
            exit(0);
        }else{
            printf("pass this pointer to lua vm  %p,\n",this);
            script::Lua::call(L,"passCobj",static_cast<void*>(&(*this)));
            script::Lua::call(L,"renewList");
            MLOG_INFO << "--------lua loaded-----";
        }
    }else{
        /*  
        if(luaL_dofile(L, spath.c_str())){
            MLOG_INFO << "ERROR:" << lua_tostring(L,-1) <<  ",main.lua not loaded,check the code";    
            //exit(0);
        }else{
        */
            script::Lua::call(L,"renewList");
            MLOG_INFO << "--------lua reloaded---";
        //}

    }
}
void CppServer::loadMap(){

}
void CppServer::start(std::string &path)
{
    cwdpath = path;
    server_.start();
    setAccessTime();
    //accessTime = std::string("");
    loadConfig();
    //reconnectDB();
    //checkRenew();
    loadLua(1);
    //todo connect ssdb
    loadMap();
    ws = WebSocket();
}

void CppServer::doLogout(const TcpConnectionPtr& conn)
{
    //std::vector<std::string> v;
    MLOG_INFO << "do cleanup:" << conn->getTcpInfoString();
    cleanup(conn);
    //retMsg(v,conn,GC_DISCONNECT); 
}

void CppServer::cleanup(const TcpConnectionPtr& conn)
{
    userIdCon.erase(conn->peerAddress().toIpPort());
    MLOG_INFO << "user id:" << conn->peerAddress().toIpPort() << " disconnect!";
    //remove conn in lua vm
    script::Lua::call(L,"removeConn",static_cast<void*>(&(*conn)));
    //
    /*
      if(msgUList[conn]){
        unsigned char* buffer=new unsigned char[24];
        unsigned char* msgws = reinterpret_cast<unsigned char*>(const_cast<char*>("disconnect"));
        ws.makeFrame(DISCONNECT_FRAME,msgws,11,buffer,0);
        std::string msg(reinterpret_cast<const char*>(buffer)); 
        //rewrite this with RAII pattern
        delete buffer;
        conn->send(msg);
        msgUList.erase(conn);
    }
    */
    connections_.erase(conn);
}

void CppServer::onConnection(const TcpConnectionPtr& conn)
{
    MLOG_INFO << conn->peerAddress().toIpPort() << " -> "
        << conn->localAddress().toIpPort() << " is "
        << (conn->connected() ? "UP" : "DOWN");
    conn->setTcpNoDelay(true);
    if (conn->connected())
    {
        connections_.insert(conn);
        MLOG_INFO << "conn int:" << conn->getTcpInfoString() ;
        userIdCon[conn->peerAddress().toIpPort()] = conn;
        //userIdCon.insert(pair<std::string,TcpConnectionPtr>(conn->peerAddress().toIpPort(),conn));
    }
    else
    {
        cleanup(conn);
    }
}

void CppServer::onMessage(const TcpConnectionPtr& conn, Buffer* buf, Timestamp)
{
    size_t len = buf->readableBytes();
    transferred_.addAndGet(len);
    receivedMessages_.incrementAndGet();
    std::string rc(buf->retrieveAllAsString());
    int msgType = MT_XOR;
    if(msgUList[conn] != MT_WEBSOCKET){
        int wsret = ws.parseHandshake(rc.c_str(),static_cast<int>(rc.length()));
        MLOG_INFO << "get ws pased ret:" << std::hex << wsret ;
        if(wsret == OPENING_FRAME){
            std::string answer(ws.answerHandshake());
            conn->send(answer);
            msgUList[conn]=MT_WEBSOCKET;
            return;
        }else{
            MLOG_INFO << "not web socket protocol." ;

        }

    }else{//found in msgUList parse incomming msg
        int out_length=0;
        int out_size=0;
        unsigned char* out=new unsigned char[rc.length()] ;
        unsigned char* inMsg = reinterpret_cast<unsigned char*>(const_cast<char *>(rc.c_str()));
        MLOG_INFO << "get ws frame";
        int wsret = ws.getFrame(inMsg,static_cast<int>(rc.length()),out,out_size,&out_length);
        switch(wsret){
            case DISCONNECT_FRAME:
                { 
                    MLOG_INFO << "get DISCONNECT_FRAME" ;
                    rc = "[10104,[\"get client DISCONNECT_FRAME\"]]";
                    msgType = MT_WEBSOCKET;

                    MLOG_INFO << "ws logout";
                    doLogout(conn);
                    return;
                }
                break; 
            case TEXT_FRAME:
                {
                    MLOG_INFO << "get ws pased ret:" << std::hex << wsret ;
                    MLOG_INFO << "out length after:" << out_length ;
                    MLOG_INFO << "get frame:"  << out ;
                    rc = std::string(reinterpret_cast<const char*>(out));
                    //rewrite this with RAII pattern
                    delete out;
                    msgType = MT_WEBSOCKET;
                }
                break;
            case PING_FRAME:
                {
                    MLOG_INFO << "ping frame";
                }
                break;
            default:
                {
                    MLOG_INFO << "other frame";
                }
        }

    }
    // muduo::StringPiece sp = muduo::StringPiece(rc);
    //MLOG_INFO << "get muduo string:" <<  rc;
    //rc = encryptDecrypt(rc,mntecsecret);
    //MLOG_INFO << "get muduo string decoded:" << rc ;
    boost::algorithm::trim(rc);
    if(rc.length() == 0) return;
    Timestamp st = Timestamp::now();
    std::vector<std::string> v;
    MLOG_INFO << "gonna call MsgDispatch:" << msgType << "," << rc;
    v = script::Lua::getRet(L,"MsgDispatch",static_cast<void*>(&(*conn)),msgType,rc.c_str());
    Timestamp et = Timestamp::now();
    double time = timeDifference(et, st);
    MLOG_INFO << "lua process time:" << time;
    if(v.size()==3){
        MLOG_INFO << "get lua retMsg:" ;
        retMsg(v,conn,0); 
    }
}

void CppServer::msghdl(const TcpConnectionPtr& conn,char *msg){
    conn->send(std::string(msg)); 
}
void CppServer::retMsg(std::vector<std::string>& v,const TcpConnectionPtr& conn,int packetID){
    MLOG_INFO << "in retMsg called";
    int i = 0;
    int retID = 0;
    std::string retIPPort;
    std::string retStr;
    int flag =0 ;
    //0:packetID 1:ipPort 2:return string
    for(std::vector< std::string >::iterator it = v.begin();it != v.end();++it)
    {
        //cout << "from lua:" << i <<  ":" <<  *it << endl;
        /*
           if(i==0) {
           if(!(stringstream(*it) >> retID)){
           retID=0;
           }
           }
           */
        if(i==1) retIPPort = *it;
        if(i==2) retStr = *it; 
        if(i==3) std::stringstream(*it) >> flag;
        if(retStr == "") {
            retID = 0;//should not happen,at least same msg copy
        }
        i++;
    }
    //retStr = encryptDecrypt(retStr,mntecsecret);

    //get packet id  and extract data from stream
    //pcrepp::Pcre pp = pcrepp::Pcre("\"",PCRE_GLOBAL);
    //sstr = pp.replace(sstr,"\\\"");
    //std::stringstream ss(rc);
    switch(retID)
    {
        case GC_BROADCAST:
            {
                /*
                   for (ConnectionList::iterator it = connections_.begin();
                   it != connections_.end();
                   ++it)
                   {
                   if((*it)!=conn){
                   (*it)->send(retStr);
                   }
                   }
                   */
            }
            break;
        case GC_DISCONNECT:
            {
                for(UserIdConList::iterator it = userIdCon.begin();it!= userIdCon.end();++it)
                {
                    it->second->send(retStr);
                }
            }
            break;
        case  0:
            {
                MLOG_INFO << "get muduo string when error:" << retStr  ;
                //conn->send(std::string("[0,['error']]"));
            }
            break;
        default:
            {
                if(flag == TYPE_GZIP){
                    std::stringstream ss(retStr);
                    boost::iostreams::filtering_streambuf<boost::iostreams::input> inbuf;
                    inbuf.push(boost::iostreams::gzip_compressor());
                    inbuf.push(ss);
                    std::stringstream ostr;
                    boost::iostreams::copy(inbuf,ostr);
                    conn->send(ostr.str());
                }else{
                    conn->send(retStr);
                }
            }
    }

}
void CppServer::setAccessTime()
        {
            struct stat st;
            st = getAccessTime(); 
            accessTime = ctime(&st.st_atime);
        }
void CppServer::checkRenew()
        {
            struct stat st = getAccessTime();
            if(accessTime != ctime(&st.st_atime)){
                MLOG_INFO << "file touched:" << ctime(&st.st_atime) ;
                accessTime = ctime(&st.st_atime);
                loadLua();
            }
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
void CppServer::cronJob()
        {
            Timestamp curTime = Timestamp::now();
            script::Lua::call(L,"TimerDispatch",static_cast<double>(curTime.microSecondsSinceEpoch()));
            
            //MLOG_INFO <<  "cronjob runs at:" << static_cast<double>(curTime.microSecondsSinceEpoch()) ;
        }
void CppServer::printThroughput()
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
            for(UserIdConList::iterator it = userIdCon.begin(); it != userIdCon.end();++it){
                it->second->send("{protocol:00001}");
            }
        }
