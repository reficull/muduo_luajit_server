module(..., package.seeall)
__index = function(t, k)
    assert(k)
	assert(nil, k)
end
__newindex = __index
__call = function(t, k, v)
    assert(type(rawget(t, k)) == type(v))
    rawset(t, k, v)
end
function New(self, arg1, arg2)
	o = {}
	Init(o, arg1, arg2)
	setmetatable(o, self)
	return o
end