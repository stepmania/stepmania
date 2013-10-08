local t = Def.ActorFrame {}

local Metric = function(c,k) return tonumber(THEME:GetMetric(c, k)) end
local width = Theme.MusicWheelItemWidth()
local height = Metric("MusicWheel", "ItemHeight")
local offset = Metric("MusicWheel", "ItemOffset")

-- background top
t[#t+1] = Def.Quad {}
t[#t].InitCommand = function(self)
	self:x(offset)
	self:vertalign(bottom)
	self:setsize(width, height/2)
	self:diffuse({0.2, 0.2, 0.2, 0.6})
end

-- background bottom
t[#t+1] = Def.Quad {}
t[#t].InitCommand = function(self)
	self:x(offset)
	self:vertalign(top)
	self:setsize(width, height/2)
	self:diffusebottomedge({0.3, 0.3, 0.3, 0.6})
	self:diffusetopedge({0.2, 0.2, 0.2, 0.6})
end

t[#t+1] = Def.Quad {}
t[#t].InitCommand = function(self)
	self:setsize(width, height * 0.8)
	self:diffuse({0, 0, 0, 0.65})
end

return t
