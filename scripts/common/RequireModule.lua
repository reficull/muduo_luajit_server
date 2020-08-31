module(..., package.seeall)
local PacketID = require("common.PacketID")
local Register = require("common.Dispatcher").Register
local CommonDefine = require("common.CommonDefine")

local function TransProtoName(str)
	res = str:sub(1, 2) .. "_"
	for i = 3, #str do
		if str:byte(i, i) < 97 and 96 < str:byte(i - 1, i - 1) then
			res = res .. "_"
		end
		res = res .. str:sub(i, i):upper()
	end
	return res
end

local function TransEventName(str)
    local A = 65; local Z = 90; local a = 97; local z = 122;
    local handleStr = "Handle"
	local res = str:sub(handleStr:len() + 1)
	local starter = 1
	local tokens = {}
	tokens[1] = "EVENT"
	for i = 1, #res-1 do
        local prevLower = res:byte(i, i) <= z and res:byte(i, i) >= a
        local nextUpper = res:byte(i+1, i+1) <= Z and res:byte(i+1, i+1) >= A
        if prevLower and nextUpper then
            tokens[#tokens+1] = res:sub(starter, i):upper()
            starter = i+1
        end
	end
	tokens[#tokens+1] = res:sub(starter):upper()

	return table.concat(tokens, "_")
end

function RegisterOneModuleProtos(moduleName, isRobot)
    print("registering proto:",moduleName)
	local EventHandler = require(moduleName .. ".EventHandler")
	if isRobot then
		EventHandler = require("robot.EventAI")
	end
	local Protocol = require(moduleName .. ".Protocol")
    --PrintTable(Protocol)
	for k, v in pairs(Protocol) do
		if k:sub(1, 2) == "CG" or k:sub(1, 2) == "GC" or k:sub(1, 2) == "GG" then
			local protoName = TransProtoName(k)
            --print("protoName:",protoName)
			assert(PacketID[protoName], protoName .. " not exist")
			assert(isRobot or k:sub(1, 2) == "GC" or k:sub(1, 2) == "GG" or EventHandler["On" .. k] or k == "CGMove" or k == "CGStopMove", "On" .. k .. " not exist")
			Register(PacketID[protoName], v, protoName, EventHandler["On" .. k])
		end
	end
	--Check for event function: HandleXXXXX() which is defined in CommonDefine.EVENT_XXXX
	for k, v in pairs(EventHandler) do
	    if k:sub(1,6) == "Handle" then
            local eventName = TransEventName(k)
            local eventValue = CommonDefine[eventName]
            assert(eventValue, "Event: "..eventName.." of function ".. k.. " not found in CommonDefine")
	    end
	end

	local ret, err = pcall(require, moduleName .. ".__init__")
	if not ret and not err:find(".__init__' not found:") then
		print(err)
	end
end
