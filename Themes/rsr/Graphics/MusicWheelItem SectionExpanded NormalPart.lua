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
	self:diffuse({0.1, 0.1, 0.1, 0.6})
end

-- background bottom
t[#t+1] = Def.Quad {}
t[#t].InitCommand = function(self)
	self:x(offset)
	self:vertalign(top)
	self:setsize(width, height/2)
	self:diffusebottomedge({0.2, 0.2, 0.2, 0.6})
	self:diffusetopedge({0.1, 0.1, 0.1, 0.6})
end

return t
