module(..., package.seeall)

CGChat = {
	{"msgType",		"SHORT",	  1},		-- 聊天类型
    {"msg",			"CHAR",		  256},		-- 聊天信息
}

GCChat = {
	{"msgType",   "SHORT",	  1},		    -- 聊天类型
    {"rolename",	"CHAR",       64},		    -- 角色名字
    {"objId",   	"INT",       1},		    -- 角色ID
    {"sex",   	    "CHAR",       1},		    -- 角色性别
    {"vipType",   	"CHAR",       1},		    -- VIP类型
    {"zhenyin",   	"CHAR",       1},		    -- 阵营
    {"title",   	"CHAR",       1},		    -- 称号
    {"msg",			"CHAR",       256},		-- 返回信息
    {"time",        "INT",       1},		-- 返回信息

}

--广播
GCBroadcast = {
    {'pos',         'CHAR',       1},
    {'msg',         'CHAR',       256},
}

