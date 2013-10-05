local t = Def.ActorFrame {}

local width = 330
local height = 46
local offset = -16

t.InitCommand=function(self)
	--self:y(0.5)
end

-- background
t[#t+1] = Def.Quad {}
t[#t].InitCommand = function(self)
	self:x(offset + 29)
	self:setsize(width - 50, height)
	self:diffuse({0, 0, 0, 0.6})
end

-- genre gen <3
local function GenerateRandomColors(song)
	if not song then return 0, {} end

	-- randomseed truncates float values, but we need more randomness.
	-- move the number over several digits to compensate.
	local length = song:MusicLengthSeconds()

	 -- BMS files without BGM mess with this (zero). Use 120 instead.
	if length == 0 then
		length = 120
	elseif length == 105 then  -- ITG length hack
		length = 1001
	end
	local seed = 10000*length
	math.randomseed(seed)

	-- Vary the amount of words returned, based on the seed.
	local rand = math.random()

	-- chances for word amount:
	-- 2/10 for one, 7/10 for two, 1/10 for three
	if rand < 0.2 then num = 1
	elseif rand > 0.9 then num = 3
	else num = 2 end

	colors = {}
	for i=1,num do
		colors[i] = {}
		math.randomseed(seed)
		colors[i][1] = math.random();
		math.randomseed(seed+5)
		colors[i][2] = math.random();
		math.randomseed(seed+10)
		colors[i][3] = math.random();
		math.randomseed(seed+15)
		colors[i][4] = clamp(math.random(), 0.75, 1.0)
		seed = seed + 1
	end
	return num, colors
end

local t2 = Def.ActorFrame {
	SetMessageCommand = function(self, params)
		local song = params.Song

		local num, colors = GenerateRandomColors(song)

		local children = {}
		for i=1,self:GetNumChildren() do
			children[i] = self:GetChild("MusicWheel_Quad"..i)
		end
		for k, v in pairs(children) do
			v:visible(false)
			if k <= num and song then
				v:visible(true)
				v:y(math.ceil(-height/2 + ((height-1)/num)*(k-1)))
				local sub = num % 2 == 0 and 1 or 0
				v:setsize(50, math.ceil((height-sub)/num))
				v:diffuse(colors[k])
			end
		end
	end
}

for i=1,3 do
	t2[#t2+1] = Def.Quad {
		Name="MusicWheel_Quad"..i,
		InitCommand=function(self)
			self:horizalign(left)
			self:vertalign(top)
			self:y(math.ceil(-height/2))
			self:x(-width/2 + offset)
			self:setsize(50, height-1)
			self:diffusealpha(0.0)
		end
	}
end

t[#t+1] = t2

t[#t+1] = LoadActor(THEME:GetPathG("", "_shigu"))
t[#t].InitCommand = cmd(visible,false;x,-width/2+3;zoom,0.25;animate,false)
t[#t].SetMessageCommand = function(self, params)
	local song = params.Song

	if song and string.lower(song:GetDisplayMainTitle()) == "sigsig" then
		self:visible(true)
		if params.HasFocus then
			self:animate(true)
		else
			self:setstate(0)
			self:animate(false)
		end
	else
		self:visible(false)
	end
end


t[#t+1] = LoadActor(THEME:GetPathG("", "_hax"))
t[#t].InitCommand = cmd(visible,false;x,-width/2-offset/2;zoom,0.25;animate,false)
t[#t].SetMessageCommand = function(self, params)
	local song = params.Song

	if song and song:MusicLengthSeconds() == 105 then
		self:visible(true)
	else
		self:visible(false)
	end
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
