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

-- bottom border
t[#t+1] = Def.Quad {}
t[#t].InitCommand = function(self)
	self:x(offset+10)
	self:y(math.ceil(height/2)+1)
	self:setsize(width+20, 2)
	self:diffuse({0.0, 0.0, 0.0, 0.8})
end

-- left border
t[#t+1] = Def.Quad {}
t[#t].InitCommand = function(self)
	self:x(-width/2+offset)
	self:y(1)
	self:setsize(2, height+2)
	self:diffuse({0.0, 0.0, 0.0, 0.8})
end

return t
