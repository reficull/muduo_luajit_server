module(...,package.seeall)
--
----定义规范：
----PacketID/100 为协议组，每个模块对应一组，如角色，技能等
----PacketID%100 为子协议号
---- 10000以下为 内部通信消息
-- CG == client to game server
-- GC == game server  to client

--- character ---
CG_ASK_LOGIN = 10001
GC_ASK_LOGIN = 10002
CG_REGISTER = 10003
GC_REGISTER = 10004

GC_HUMAN_INFO = 10006

CG_GET_NAME = 10052
GC_GET_NAME = 10053

CG_CHECK_ACCNAME = 10054
GC_CHECK_ACCNAME = 10055

CG_CREATE_ROLE = 10056
GC_CREATE_ROLE = 10057

--- chat ---
CG_CHAT = 10101
GC_CHAT = 10102
GC_BROADCAST  = 10103
CG_DISCONNECT = 10104
GC_DISCONNECT = 10105

--- http request ---
CG_GET = 20101
GC_GET = 20102

