#ifndef _DEFINE_PCH
#define _DEFINE_PCH


#define CONFIGFILE "config.lua"
#define ONLINESTATUS 2
#define OFFLINESTATUS 1

#define MLOG_INFO if (muduo::Logger::logLevel() <= muduo::Logger::INFO) \
                                                   muduo::Logger(__FILE__, __LINE__).stream()

#endif
