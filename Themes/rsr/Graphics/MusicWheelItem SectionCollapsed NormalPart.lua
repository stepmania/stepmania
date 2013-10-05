local t = Def.ActorFrame {}

local width = 330
local height = 46
local offset = -16

-- background top
t[#t+1] = Def.Quad {}
t[#t].InitCommand = function(self)
	self:x(offset)
	self:vertalign(bottom)
	self:setsize(width, height/2)
	self:diffusetopedge({0.05, 0.05, 0.05, 0.6})
	self:diffusebottomedge({0.0, 0.0, 0.0, 0.6})
end

-- background bottom
t[#t+1] = Def.Quad {}
t[#t].InitCommand = function(self)
	self:x(offset)
	self:vertalign(top)
	self:setsize(width, height/2)
	self:diffuse({0, 0, 0, 0.6})
end

return t
