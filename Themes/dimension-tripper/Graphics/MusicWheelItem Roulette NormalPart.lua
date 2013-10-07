local t = Def.ActorFrame {}

local Metric = function(c,k) return tonumber(THEME:GetMetric(c, k)) end
local width = Theme.MusicWheelItemWidth()
local height = Metric("MusicWheel", "ItemHeight")
local offset = Metric("MusicWheel", "ItemOffset")

-- background mask
t[#t+1] = Def.Quad {}
t[#t].InitCommand = function(self)
	self:x(offset)
	self:setsize(width, height)
	self:diffuse({0, 0, 0, 0.6})
end

t[#t+1] = LoadActor(THEME:GetPathG("", "_texture checkerboard"))
t[#t].InitCommand = function(self)
	self:rainbow()
	self:diffusealpha(0.05)
	self:stretchto(-width/2 + offset, -height/2, width/2 + offset, height/2)
	self:customtexturerect(0, 0, width/32, height/32)
	self:texcoordvelocity(0.5, 0.25)
end

t[#t+1] = LoadActor(THEME:GetPathG("", "_texture checkerboard"))
t[#t].InitCommand = function(self)
	self:rainbow()
	self:diffusealpha(0.05)
	self:stretchto(-width/2 + offset, -height/2, width/2 + offset, height/2)
	self:customtexturerect(0, 0, width/32, height/32)
	self:texcoordvelocity(0.5, -0.25)
end

t[#t+1] = LoadActor(THEME:GetPathG("", "_texture checkerboard"))
t[#t].InitCommand = function(self)
	self:rainbow()
	self:diffusealpha(0.05)
	self:stretchto(-width/2 + offset, -height/2, width/2 + offset, height/2)
	self:customtexturerect(0, 0, width/32, height/32)
	self:texcoordvelocity(-0, 0.5)
end

return t
