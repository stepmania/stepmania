--[[
Custom Speed Mods v2.0 (for sm-ssc)
by AJ Kelly of KKI Labs ( http://kki.ajworld.net/ )

changelog:

v2.0 (for sm-ssc)
Giant rewrite of the speed mod parser.
This rewrite comes with the following changes/features:
* Speed mods are now tied to profiles.
  This is arguably the biggest change, as it allows the speed mods to be
  portable, as well as per-profile.
  Thanks to this, we can now support reading SpeedMods from a USB stick or
  other external storage. (I didn't test writing yet, but it should work.)

* Data/SpeedMods.txt is the fallback.
  Previously, all speed mods were stored in {SM4 folder}/Data/SpeedMods.txt.
  For compatibility reasons, this file is still read by the script.

This version of Custom Speed Mods will only run on sm-ssc for the time being,
DO NOT use it in themes for StepMania 4 alpha versions.
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
________________________________________________________________________________
anticipated future changes:
* M-Mod support (when sm-ssc integrates it)
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
	local file = RageFileUtil.CreateRageFile()
	if file:Open(path, 1) then
		-- success
		local contents = file:Read()
		mods = split(',',contents)

		-- strip any whitespace
		for i=1,#mods do
			string.gsub(mods[i], "%s", "")
		end

		file:destroy()
		return mods
	else
		-- error; write a fallback mod file and return it
		local fallbackString = "0.5x,0.75x,1x,1.75x,2x,2.25x,2.5x,C150,C300"
		Trace("[CustomSpeedMods]: Could not read SpeedMods; writing fallback to "..path)
		file:Open(path, 2)
		file:Write(fallbackString)
		file:destroy()
		return split(',',fallbackString)
	end
end

-- MarkDupes(src,parent)
-- Marks duplicates in src from any matches in parent.
-- the overall mods are usually used as the parent.
local function MarkDupes(src,parent)
	for iPar=1,#parent do
		for iSrc=1,#src do
			if parent[iPar] == src[iSrc] then
				src[iSrc] = "XXX"
			end
		end
	end
	return src
end

-- RemoveMarked(src)
-- Removes any values marked for deletion.
local function RemoveMarked(src)
	for iSrc=1,#src do
		if src[iSrc] == "XXX" then
			table.remove(src,iSrc)
		end
	end
	return src
end

-- MergeTables(parent,child)
-- Adds the child's contents to the parent.
-- the overall mods are usually used as the parent.
local function MergeTables(parent,child)
	child = RemoveMarked(child)
	if #child == 0 then
		return parent
	end

	local addMe = true
	for iC=1,#child do
		--[[
		for iP=1,#parent do
			if addMe then
				-- check if that's the case.
				-- why am I doing this anyways?
				-- by the time these tables are passed in,
				-- dupes should be gone.
			end
		end
		]]
		if addMe then
			table.insert(parent,child[iC])
		end
	end
	return parent
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
	local cMods = {}
	--local mMods = {}

	-- convert to numbers so sorting works:
	for i=1,#tab do
		local typ,val
		-- xxx: If people use a floating point CMod (e.g. C420.50),
		-- it will get rounded. C420.50 gets rounded to 421, btw. -aj
		if string.find(tab[i],"C%d") then
			typ = cMods
			val = string.gsub(tab[i], "C", "")
		elseif string.find(tab[i],"M%d") then
			Trace("[CustomSpeedMods] OpenITG's M-Mods are not supported yet in sm-ssc.")
			--typ = mMods
			--val = string.gsub(tab[i], "M", "")
		else
			typ = xMods
			val = string.gsub(tab[i], "x", "")
		end
		table.insert(typ,tonumber(val))
	end

	-- sort xMods
	xMods = AnonSort(xMods)
	-- sort cMods
	cMods = AnonSort(cMods)
	-- sort mMods
	--mMods = AnonSort(mMods)
	local fin = {}
	-- convert it back to a string since that's what it expects
	for i=1,#xMods do
		table.insert(fin, xMods[i].."x")
	end
	for i=1,#cMods do
		table.insert(fin, "C"..cMods[i])
	end
	--for i=1,#mMods do table.insert(fin, "M"..mMods[i]); end;
	return fin
end

-- parse everything
local function GetSpeedMods()
	local finalMods = {}

	local baseFilename = "SpeedMods.txt"
	local profileDirs = {
		Fallback = "Data/",
		Machine = ProfileDir('ProfileSlot_Machine'),
		PlayerNumber_P1 = ProfileDir('ProfileSlot_Player1'),
		PlayerNumber_P2 = ProfileDir('ProfileSlot_Player2')
	}

	-- figure out how many players we have to deal with.
	local numPlayers = GAMESTATE:GetNumPlayersEnabled()
	-- load fallback
	local fallbackMods = ParseSpeedModFile(profileDirs.Fallback..baseFilename)

	-- load machine
	local machineMods = ParseSpeedModFile(profileDirs.Machine..baseFilename)

	local playerMods = {}
	for pn in ivalues(GAMESTATE:GetHumanPlayers()) do
		-- file loading logic per player;
		-- only bother if it's not the machine profile though.
		if PROFILEMAN:IsPersistentProfile(pn) or
			MEMCARDMAN:GetCardState(pn) == 'MemoryCardState_ready' then
			playerMods[#playerMods+1] = ParseSpeedModFile(profileDirs[pn]..baseFilename)
		end
	end

	-- with all loaded... the merging BEGINS!!
	finalMods = fallbackMods
	-- mine for duplicates, first pass (fallback <-> machine)
	machineMods = MarkDupes(machineMods,finalMods)
	for ply=1,#playerMods do
		playerMods[ply] = MarkDupes(playerMods[ply],finalMods)
	end
	-- remove XXX, first pass
	machineMods = RemoveMarked(machineMods);
	for ply=1,#playerMods do
		playerMods[ply] = RemoveMarked(playerMods[ply])
	end
	-- mine for duplicates, second pass (machine <-> player)
	for ply=1,#playerMods do
		playerMods[ply] = MarkDupes(playerMods[ply],machineMods)
	end
	-- remove XXX, second pass
	machineMods = RemoveMarked(machineMods)
	for ply=1,#playerMods do
		playerMods[ply] = RemoveMarked(playerMods[ply])
	end

	-- merge zone
	finalMods = MergeTables(finalMods,machineMods)
	for ply=1,#playerMods do
		finalMods = MergeTables(finalMods,playerMods[ply])
	end

	-- final removal of XXX before sorting
	finalMods = RemoveMarked(finalMods)
	-- sort the mods before returning them
	return SpeedModSort(finalMods)
end

function SpeedMods()
	-- here we see the option menu itself.
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
Copyright © 2008-2009 AJ Kelly/KKI Labs.
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