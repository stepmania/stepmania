--[[
Custom Speed Mods v2.3 (for StepMania 5/SM5TE)
by AJ Kelly of KKI Labs ( http://kki.ajworld.net/ )

changelog:

v2.3 (StepMania 5 a2/SM5TE)
* If someone has decided to remove 1x from the machine profile's speed mods,
  silently fix it.
* Ignore Cmod and mmod capitalization errors.

v2.2 (StepMania 5 alpha 2) [by FSX]
* Rewrite table management code.
* Add code to make sure that there are speed mods and that they are correct.

v2.1 (StepMania 5 Preview 2)
* Added support for m-Mods.

v2.0 (for sm-ssc)
Giant rewrite of the speed mod parser.
This rewrite comes with the following changes/features:
* Speed mods are now tied to profiles.
  This is arguably the biggest change, as it allows the speed mods to be
  portable, as well as per-profile.
  Thanks to this, we can now support reading SpeedMods from a USB stick or
  other external storage. (I didn't test writing yet, but it should work.)

This version of Custom Speed Mods will only run on StepMania 5 (due to m-mods).
--------------------------------------------------------------------------------
v1.4
* Try to auto-set the speed mod to 1.0 if:
 1) The player hasn't already chosen a speed mod
 2) The player's custom speed mod collection starts with a value under 1x.
 Due to the way the custom speed mods were coded, it will always pick the
 first value, even if it's not 1.0x.

v1.3
* strip whitespace out of file in case people use it.
	(I don't think it really works but SM seems to think the mods are legal)
* fixed an error related to using the fallback return value.

v1.2
* small fixes
* more comments

v1.1
* Cleaned up code some, I think.
]]

-- ProfileDir(slot): gets the profile dir for slot,
-- where slot is a 'ProfileSlot_*' enum value.
local function ProfileDir(slot)
	local profileDir = PROFILEMAN:GetProfileDir(slot)
	return profileDir or nil
end

-- Tries to parse the file at path. If successful, returns a table of mods.
-- If it can't open the file, it will write a fallback set of mods.
local function ParseSpeedModFile(path)
	local function Failure()
		-- error; write a fallback mod file and return it
		local fallbackString = "0.5x,1x,1.5x,2x,3x,4x,5x,6x,7x,8x,C250,C450,m550"
		Trace("[CustomSpeedMods]: Could not read SpeedMods; writing fallback to "..path)
		local file = RageFileUtil.CreateRageFile()
		file:Open(path, 2)
		file:Write(fallbackString)
		file:destroy()
		return split(',',fallbackString)
	end

	local file = RageFileUtil.CreateRageFile()
	if file:Open(path, 1) then
		-- success
		local contents = file:Read()
		mods = split(',',contents)
		
		-- strip any whitespace and check 
		for i=1,#mods do
			string.gsub(mods[i], "%s", "")
			if not(mods[i]:find("%d+.?%d*[xX]") or mods[i]:find("[cmCM]%d+")) then
				mods[i] = nil
			elseif mods[i]:find("[mM]") then mods[i]=mods[i]:lower()
			elseif mods[i]:find("[cC]") then mods[i]=mods[i]:upper() end
		end
		
    if #mods==0 then file:destroy() return Failure() end
		
		file:destroy()
		return mods
	else
		file:destroy()
		return Failure()
	end
end

-- InvertTable(tbl)
-- Returns a table where a key-value pair is swapped.
local function InvertTable(tbl)
	local rTable = {}
	for k,v in pairs(tbl) do
		rTable[v] = k
	end
	return rTable
end

-- MergeTables(parent,child)
-- Puts two tables together, overwriting values
-- with the same key.
local function MergeTables(parent, child)
	for k,v in pairs(parent) do
		child[k] = v
	end
	return child
end

-- code in this function is based off of code in
-- http://astrofra.com/weblog/files/sort.lua
local function AnonSort(t)
	local index_min
	for i=1,#t,1 do
		index_min = i
		for j=i+1,#t,1 do
			if (t[j] < t[index_min]) then
				index_min = j
			end
		end
		t[i], t[index_min] = t[index_min], t[i]
	end
	return t
end

local function SpeedModSort(tab)
	local xMods = {}
	local mMods = {}
	local cMods = {}

	-- convert to numbers so sorting works:
	for i=1,#tab do
		local typ,val
		-- xxx: If people use a floating point CMod (e.g. C420.50),
		-- it will get rounded. C420.50 gets rounded to 421, btw. -aj
		if string.find(tab[i],"C%d") then
			typ = cMods
			val = string.gsub(tab[i], "C", "")
		-- support both cases because I want to hit people -freem
		elseif string.find(tab[i],"m%d") or string.find(tab[i],"M%d") then
			typ = mMods
			val = string.gsub(tab[i], "m", "")
		else
			typ = xMods
			val = string.gsub(tab[i], "x", "")
		end
		table.insert(typ,tonumber(val))
	end

	-- sort Mods
	xMods = AnonSort(xMods)
	cMods = AnonSort(cMods)
	mMods = AnonSort(mMods)
	local fin = {}
	-- convert it back to a string since that's what it expects
	for i=1,#xMods do
		table.insert(fin, xMods[i].."x")
	end
	for i=1,#cMods do
		table.insert(fin, "C"..cMods[i])
	end
	for i=1,#mMods do
		table.insert(fin, "m"..mMods[i])
	end
	return fin
end

-- parse everything
local function GetSpeedMods()
	local finalMods = {}

	local baseFilename = "SpeedMods.txt"
	local profileDirs = {
		Machine = ProfileDir('ProfileSlot_Machine'),
		PlayerNumber_P1 = ProfileDir('ProfileSlot_Player1'),
		PlayerNumber_P2 = ProfileDir('ProfileSlot_Player2')
	}

	-- if someone is trying to be "smart" and removes 1x from the machine
	-- profile's custom speed mods, re-add it to prevent crashes.
	local machineMods = ParseSpeedModFile(profileDirs.Machine..baseFilename)
	if machineMods then
		local found1X = false
		for k,v in pairs(machineMods) do found1X = (v == "1x") end
		if not found1X then table.insert(machineMods,"1x") end
	end

	-- load machine to finalMods
	finalMods = InvertTable(machineMods)

	-- figure out how many players we have to deal with.
	local numPlayers = GAMESTATE:GetNumPlayersEnabled()

	local playerMods = {}
	for pn in ivalues(GAMESTATE:GetHumanPlayers()) do
		-- file loading logic per player.
		if PROFILEMAN:IsPersistentProfile(pn) then
			playerMods[#playerMods+1] = ParseSpeedModFile(profileDirs[pn]..baseFilename)
		-- we need to make sure the memory card is ready if we're gonna grab from it.
		elseif MEMCARDMAN:GetCardState(pn) == 'MemoryCardState_ready' then
			playerMods[#playerMods+1] = ParseSpeedModFile(profileDirs[pn]..baseFilename)
		end
	end
	
	-- join players, overwriting duplicates
	for ply=1,#playerMods do
		finalMods=MergeTables(finalMods,InvertTable(playerMods[ply]))
	end
	
	-- convert into an unsorted integer-indexed table
	do
		local curIndex = 1
		local newFinalMods = {}
		for k,v in pairs(finalMods) do
			newFinalMods[curIndex] = k
			curIndex = curIndex + 1
		end
		finalMods = newFinalMods
	end
	
	-- sort the mods before returning them
	return SpeedModSort(finalMods)
end

function SpeedMods()
	local t = {
		Name = "Speed",
		LayoutType = "ShowAllInRow",
		SelectType = "SelectOne",
		OneChoiceForAllPlayers = false,
		ExportOnChange = false,
		Choices = GetSpeedMods(),

		LoadSelections = function(self, list, pn)
			local pMods = GAMESTATE:GetPlayerState(pn):GetPlayerOptionsString("ModsLevel_Preferred")
			for i = 1,table.getn(self.Choices) do
				if string.find(pMods, self.Choices[i]) then
					list[i] = true
					return
				end
			end

			-- if we've reached this point, try to find 1x or 1.0x instead,
			-- in case the player has defined a speed mod under 1.0x
			for i = 1,table.getn(self.Choices) do
				if self.Choices[i] == "1x" or self.Choices[i] == "1.0x" then
					list[i] = true
					return
				end
			end
		end,
		SaveSelections = function(self, list, pn)
			for i = 1,table.getn(self.Choices) do
				if list[i] then
					local PlayerState = GAMESTATE:GetPlayerState(pn)
					PlayerState:SetPlayerOptions("ModsLevel_Preferred",self.Choices[i])
					return
				end
			end
		end
	}
	setmetatable( t, t )
	return t
end

--[[
Copyright © 2008-2012 AJ Kelly/KKI Labs.
Use freely, so long this notice and the above documentation remains.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
]]