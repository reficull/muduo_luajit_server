module(..., package.seeall)
local ffi      = require 'ffi'

getcwd = function(buf, size) return ffi.string( ffi.C.getcwd(buf, size) ) end
local function basepath()
  local buf = ffi.new("char[256]")
  return (getcwd(buf, 256).."/") -- get working directory with trailing / 
end
wsjit = assert(ffi.load("/usr/local/lib/libwsjit.so"),"load libwsjit failed")
--wsjit = assert(ffi.load(basepath().. 'build/release/lib/libwsjit.so'),"load libwsjit.so failed")
ffi.cdef[[
    typedef struct WebSocket wsjit;
    wsjit* ws_new();
    void ws_gc(void *this_);

    enum WebSocketFrameType {
	    ERROR_FRAME=0xFF00,
	    INCOMPLETE_FRAME=0xFE00,

	    OPENING_FRAME=0x3300,
	    CLOSING_FRAME=0x3400,

	    INCOMPLETE_TEXT_FRAME=0x01,
	    INCOMPLETE_BINARY_FRAME=0x02,

	    TEXT_FRAME=0x81,
	    BINARY_FRAME=0x82,

	    PING_FRAME=0x19,
	    PONG_FRAME=0x1A
    };
    
 	WebSocketFrameType ws_parseHandshake(wsjit* this_,unsigned char* input_frame, int input_len);   
    const char* ws_answerHandshake(wsjit* this_);
    int ws_makeFrame(wsjit* this_,WebSocketFrameType frame_type, unsigned char* msg, int msg_len, unsigned char* buffer, int buffer_len);
    WebSocketFrameType ws_getFrame(wsjit* this_, unsigned char* in_buffer, int in_length, unsigned char* out_buffer, int out_size, int* out_length);

]]

local ptr_chs   = ffi.typeof("char const*")
local up_chs    = ffi.typeof("unsigned char*")
local wstype    = ffi.typeof("WebSocketFrameType")
--[[
local WebSocketFrameType = {
    ERROR_FRAME=0xFF00,
    INCOMPLETE_FRAME=0xFE00,

    OPENING_FRAME=0x3300,
    CLOSING_FRAME=0x3400,

    INCOMPLETE_TEXT_FRAME=0x01,
    INCOMPLETE_BINARY_FRAME=0x02,

    TEXT_FRAME=0x81,
    BINARY_FRAME=0x82,

    PING_FRAME=0x19,
    PONG_FRAME=0x1A
}
]]--
local mt = {}
mt.__index = mt

function mt.ws_makeFrame(self,frame_type,msg,msg_len,buffer,buffer_len)
   local r = wsjit.ws_makeFrame(self.super,ffi.new(wstype,frame_type),ffi.cast(up_chs,msg),ffi.new("int",string.len(msg)),ffi.cast(up_chs,buffer),ffi.new("int",string.len(buffer)))
   return r
end

function mt.ws_getFrame(self,in_buffer,in_length,out_buffer,out_size,out_length)
    local rl = ffi.cast(ffi.typeof("int*")) 
   local r = wsjit.ws_getFrame(self.super,ffi.cast(up_chs,in_buffer),ffi.new("int",in_length),ffi.cast(up_chs,out_buffer),ffi.new(ffi.typeof("int"),out_size),rl) 

end

function mt.parseHandshake(self,input_frame)
    local f = ffi.cast(up_chs,input_frame)
    local l = string.len(input_frame)
    local l = ffi.new("int",l)
    f = input_frame
    
    wstype = wsjit.ws_parseHandshake(self.super,f,l)
    return wstype
end

function mt.answerHandshake(self)
    local r = ffi.string(ffi.cast(ptr_chs,wsjit.ws_answerHandshake(self.super)))
    return r
end

