local t = Def.ActorFrame {}

local Metric = function(c,k) return tonumber(THEME:GetMetric(c, k)) end
local width = Theme.MusicWheelItemWidth()
local height = Metric("MusicWheel", "ItemHeight")
local offset = Metric("MusicWheel", "ItemOffset")

-- genre gen <3
local function GenerateRandomColors(song)
	if not song then return 0, {} end

	-- randomseed truncates float values, but we need more randomness.
	-- move the number over several digits to compensate.
	local length = song:MusicLengthSeconds()

	if song:GetDisplayMainTitle() == "DVNO" then
		return 2, { { 1, 0.8, 0, 1 }, { 1, 0.6, 0, 1 } }
	end

	if song:GetDisplayMainTitle() == "The Witches' Ball" then
		return 3, { { 0.2, 0.1, 0.3, 1 }, { 0.7, 0.7, 1.0, 1 }, { 0.2, 0.1, 0.3, 1 } }
	end

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
				v:x(math.ceil(-width/2 + ((width-1)/num)*(k-1)))
				local sub = num % 2 == 0 and 1 or 0
				v:setsize(math.ceil((width-sub)/num), height)
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
			self:vertalign(bottom)
			self:y(math.ceil(height/2))
			self:diffusealpha(0.0)
		end
	}
end

t[#t+1] = t2

t[#t+1] = LoadActor(THEME:GetPathG("", "_texture checkerboard"))
t[#t].InitCommand = function(self)
	self:stretchto(-width/2 + offset, -height/2*0.8, width/2 + offset, height/2*0.8)
	self:customtexturerect(0, 0, width/16, height/16)
	self:texcoordvelocity(0, 0.25)
end
t[#t].SetMessageCommand = function(self, params)
		local song = params.Song
		local num, colors = GenerateRandomColors(song)

		if not colors[num] then return end
		colors[num][1] = colors[num][1] + 0.25
		colors[num][2] = colors[num][2] + 0.25
		colors[num][3] = colors[num][3] + 0.25

		self:SetTextureFiltering(false)
		colors[num][4] = 0.125
		self:blend(Blend.Add)
		self:diffuse(colors[num])
end

-- background
t[#t+1] = Def.Quad {}
t[#t].InitCommand = function(self)
	self:setsize(width, height * 0.8)
	self:diffuse({0, 0, 0, 0.7})
end

t[#t+1] = LoadActor(THEME:GetPathG("", "_shigu"))
t[#t].InitCommand = cmd(visible,false;x,width/2-32;zoom,0.25;animate,false)
t[#t].SetMessageCommand = function(self, params)
	local song = params.Song

	if song and song:GetDisplayMainTitle():lower() == "sigsig" then
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
t[#t].InitCommand = cmd(visible,false;x,width/2-32;zoom,0.25;animate,false)
t[#t].SetMessageCommand = function(self, params)
	local song = params.Song

	if song and song:MusicLengthSeconds() == 105 then
		self:visible(true)
	else
		self:visible(false)
	end
end

return t
