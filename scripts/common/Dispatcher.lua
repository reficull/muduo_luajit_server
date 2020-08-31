module(..., package.seeall)
ProtoTemplate = {}
ProtoContainer = {} --装载协议的可重用table
ProtoName = {}
ProtoHandler = {} --包含所有CG协议的handler
ProtoContainer2PacketID = {}

__index = function(t, k)
	local packetID = ProtoContainer2PacketID[t] or ""
	assert(nil, packetID .. "." .. k .. " key not exist check protocol define or init")
end
__newindex = __index

function SetProtoContainerMetatable(tb)
	setmetatable(tb, _M)
	for k, v in pairs(tb) do
		if type(v) == "table" then
			SetProtoContainerMetatable(v)
		end
	end
end

function InitProtoContainer(msg, template)
	for k, v in ipairs(template) do
		if v[3] > 1 then
			--msg[v[1] .. "Len"] = 0
			msg[v[1]] = {}
			for i = 1, v[3] do
				if type(v[2]) == "string" then
					msg[v[1]][i] = 0
				else
					msg[v[1]][i] = {}
					InitProtoContainer(msg[v[1]][i], v[2])
				end
			end
		else
			if type(v[2]) == "string" then
				msg[v[1]] = 0
			else
				msg[v[1]] = {}
				InitProtoContainer(msg[v[1]], v[2])
			end
		end
	end
end

function Register(packetID, protoTemplate, protoName, protoHandler)
	ProtoContainer[packetID] = {}
	ProtoTemplate[packetID] = protoTemplate
	ProtoName[packetID] = protoName
	ProtoHandler[packetID] = protoHandler
	ProtoContainer2PacketID[ProtoContainer[packetID]] = packetID

	InitProtoContainer(ProtoContainer[packetID], protoTemplate)
	SetProtoContainerMetatable(ProtoContainer[packetID])
	-- ProtoTemplateToTree(packetID, protoTemplate)
end

function TraceMsgDfs(msg, template, step)
	for k, v in ipairs(template) do
		if v[3] > 1 then
			print(string.rep("	", step), v[1].."Len", "=", msg[v[1].."Len"])
			if type(msg[v[1]]) == "string" then 
				print(string.rep("	", step), msg[v[1]])
			else
				for i = 1, msg[v[1].."Len"] do
					if type(v[2]) == "table" then
						TraceMsgDfs(msg[v[1]][i], v[2], step + 1)
					else
						print(string.rep("	", step), i, "=", msg[v[1]][i])
					end
				end
			end
		else
			if type(v[2]) == "table" then
				TraceMsgDfs(msg[v[1]], v[2], step + 1)
			else
				print(string.rep("	", step), v[1], "=", msg[v[1]])
			end
		end
	end
end

function TraceMsg(msg)
	local packetID = ProtoContainer2PacketID[msg]
	print(ProtoName[packetID])
	TraceMsgDfs(msg, ProtoTemplate[packetID], 0)
end

--定时器分发
TimerDispatcher = {}
function RegisterTimerHandler(nEventID, TimerHandler)
	TimerDispatcher[nEventID] = TimerHandler
end
