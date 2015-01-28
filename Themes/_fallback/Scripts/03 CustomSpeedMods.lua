--[[
Custom Speed Mods v3 (for StepMania 5)

changelog:

v3 (StepMania 5 b3)
* Complete rewrite to use profile load/save hooks.

--------------------------------------------------------------------------------
v2.3 (StepMania 5 a2/SM5TE) [by AJ]
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

local ProfileSpeedMods = {}

-- Returns a new, empty mod table: a table with three members x, C, and m,
-- each being a table with the corresponding numbers set to true.
local function EmptyModTable()
	return {x = {}, C = {}, m = {}}
end

-- Merge one mod table into another.
local function MergeInModTable(dst, src)
	for typ, subtbl in pairs(src) do
		for n, v in pairs(subtbl) do
			dst[typ][n] = v
		end
	end
end

-- Parses a speed mod and returns the pair (type, number) or nil if parsing
-- failed.
local function CanonicalizeMod(mod)
	num = tonumber(mod:match("^(%d+.?%d*)[xX]$"))
	if num ~= nil then
		return "x", num
	end

	num = tonumber(mod:match("^[cC](%d+.?%d*)$"))
	if num ~= nil then
		return "C", num
	end

	num = tonumber(mod:match("^[mM](%d+.?%d*)$"))
	if num ~= nil then
		return "m", num
	end

	return nil
end

-- Parse a comma-separated string into a mod table.
local function StringToModTable(str)
	local mods = EmptyModTable()
	local valid = false

	string.gsub(str, "%s", "")
	for _, mod in ipairs(split(",", str)) do
		local t, n = CanonicalizeMod(mod)
		if t then
			mods[t][n] = true
			valid = true
		end
	end

	return valid and mods or nil
end

-- Return the contents of a mod table as a list of mod names.
local function ModTableToList(mods)
	local l = {}
	local tmp = {}

	-- Do x-mods separately because the x comes after
	for mod, _ in pairs(mods.x) do
		table.insert(tmp, mod)
	end
	table.sort(tmp)
	for _, mod in ipairs(tmp) do
		table.insert(l, mod .. "x")
	end

	-- C- and m-mods
	for _, modtype in ipairs({"C", "m"}) do
		tmp = {}
		for mod, _ in pairs(mods[modtype]) do
			table.insert(tmp, mod)
		end
		table.sort(tmp)
		for _, mod in ipairs(tmp) do
			table.insert(l, modtype .. mod)
		end
	end

	return l
end

local DefaultMods = StringToModTable("0.5x,1x,1.5x,2x,3x,4x,5x,6x,7x,8x,C250,C450,m550")

-- Reads the custom speed mod file at <path> and returns a corresponding mod
-- table.
local function ReadSpeedModFile(path)
	local file = RageFileUtil.CreateRageFile()
	if not file:Open(path, 1) then
		file:destroy()
		return nil
	end

	local contents = file:Read()
	file:Close()
	file:destroy()

	return StringToModTable(contents)
end

-- Hook called during profile load
function LoadProfileCustom(profile, dir)
	-- This will be (intentionally) nil if the file is missing or bad
	local mods = ReadSpeedModFile(dir .. "SpeedMods.txt")

	-- Special case for the machine profile
	if profile == PROFILEMAN:GetMachineProfile() then
		ProfileSpeedMods.machine = mods
		return
	end

	-- Otherwise, it's a player profile.  Store accordingly.
	for i = 1, NUM_PLAYERS do
		if profile == PROFILEMAN:GetProfile(PlayerNumber[i]) then
			ProfileSpeedMods[PlayerNumber[i]] = mods
			break
		end
	end
end

-- Hook called during profile save
function SaveProfileCustom(profile, dir)
	-- Change this if a theme allows you to change and save custom
	-- per-profile settings.
end

-- Returns a list of speed mods for the current round.
local function GetSpeedMods()
	-- Start with machine profile
	local mods = ProfileSpeedMods.machine or EmptyModTable()

	-- Merge in any active players
	for _, p in ipairs(GAMESTATE:GetHumanPlayers()) do
		if ProfileSpeedMods[p] and PROFILEMAN:IsPersistentProfile(p) then
			MergeInModTable(mods, ProfileSpeedMods[p])
		else
			MergeInModTable(mods, DefaultMods)
		end
	end

	-- Apparently removing 1x caused crashes, so be sure it's there.
	-- (This may not be a problem anymore. -- djpohly)
	mods.x[1] = true
	return ModTableToList(mods)
end

-- Implementation of custom Lua option row
function SpeedMods()
	local t = {
		Name = "Speed",
		LayoutType = "ShowAllInRow",
		SelectType = "SelectOne",
		OneChoiceForAllPlayers = false,
		ExportOnChange = false,
		Choices = GetSpeedMods(),

		LoadSelections = function(self, list, pn)
			local pref = GAMESTATE:GetPlayerState(pn):GetPlayerOptionsString("ModsLevel_Preferred")
			local selected = 0

			for i, choice in ipairs(self.Choices) do
				if string.find(pref, choice) then
					-- Found it, use it
					selected = i
					break
				elseif choice == "1x" then
					-- Pick this unless we find the
					-- preferred choice
					selected = i
				end
			end

			-- If we didn't find a match, just use the first
			if selected ~= 0 then
				list[selected] = true
			else
				list[1] = true
			end
		end,
		SaveSelections = function(self, list, pn)
			local state = GAMESTATE:GetPlayerState(pn)
			for i, choice in ipairs(self.Choices) do
				if list[i] then
					state:SetPlayerOptions("ModsLevel_Preferred", choice)
					return
				end
			end
			-- Or use the first
			state:SetPlayerOptions("ModsLevel_Preferred", self.Choices[1])
		end
	}
	return t
end

local default_speed_increment= 25
local default_speed_inc_large= 100

local function get_speed_increment()
	local increment= default_speed_increment
	if ReadGamePrefFromFile("SpeedIncrement") then
		increment= tonumber(GetGamePref("SpeedIncrement")) or default_speed_increment
	else
		WriteGamePrefToFile("SpeedIncrement", increment)
	end
	return increment
end

local function get_speed_inc_large()
	local inc_large= default_speed_inc_large
	if ReadGamePrefFromFile("SpeedIncLarge") then
		inc_large= tonumber(GetGamePref("SpeedIncLarge")) or default_speed_inc_large
	else
		WriteGamePrefToFile("SpeedIncLarge", inc_large)
	end
	return inc_large
end

function SpeedModIncSize()
	-- An option row for controlling the size of the increment used by
	-- ArbitrarySpeedMods.
	local increment= get_speed_increment()
	local ret= {
		Name= "Speed Increment",
		LayoutType= "ShowAllInRow",
		SelectType= "SelectMultiple",
		OneChoiceForAllPlayers= true,
		LoadSelections= function(self, list, pn)
			-- The first value is the status element, only it should be true.
			list[1]= true
		end,
		SaveSelections= function(self, list, pn)
			WriteGamePrefToFile("SpeedIncrement", increment)
		end,
		NotifyOfSelection= function(self, pn, choice)
			-- return true even though we didn't actually change anything so that
			-- the underlines will stay correct.
			if choice == 1 then return true end
			local incs= {10, 1, -1, -10}
			local new_val= increment + incs[choice-1]
			if new_val > 0 then
				increment= new_val
			end
			self:GenChoices()
			return true
		end,
		GenChoices= function(self)
			self.Choices= {tostring(increment), "+10", "+1", "-1", "-10"}
		end
	}
	ret:GenChoices()
	return ret
end

function SpeedModIncLarge()
	-- An option row for controlling the size of the increment used by
	-- ArbitrarySpeedMods.
	local inc_large= get_speed_inc_large()
	local ret= {
		Name= "Speed Increment Large",
		LayoutType= "ShowAllInRow",
		SelectType= "SelectMultiple",
		OneChoiceForAllPlayers= true,
		LoadSelections= function(self, list, pn)
			-- The first value is the status element, only it should be true.
			list[1]= true
		end,
		SaveSelections= function(self, list, pn)
			WriteGamePrefToFile("SpeedIncLarge", inc_large)
		end,
		NotifyOfSelection= function(self, pn, choice)
			-- return true even though we didn't actually change anything so that
			-- the underlines will stay correct.
			if choice == 1 then return true end
			local incs= {10, 1, -1, -10}
			local new_val= inc_large + incs[choice-1]
			if new_val > 0 then
				inc_large= new_val
			end
			self:GenChoices()
			return true
		end,
		GenChoices= function(self)
			self.Choices= {tostring(inc_large), "+10", "+1", "-1", "-10"}
		end
	}
	ret:GenChoices()
	return ret
end

function GetSpeedModeAndValueFromPoptions(pn)
	local poptions= GAMESTATE:GetPlayerState(pn):GetPlayerOptions("ModsLevel_Preferred")
	local speed= nil
	local mode= nil
	if poptions:MaxScrollBPM() > 0 then
		mode= "m"
		speed= math.round(poptions:MaxScrollBPM())
	elseif poptions:TimeSpacing() > 0 then
		mode= "C"
		speed= math.round(poptions:ScrollBPM())
	else
		mode= "x"
		speed= math.round(poptions:ScrollSpeed() * 100)
	end
	return speed, mode
end

function ArbitrarySpeedMods()
	-- If players are allowed to join while this option row is active, problems will probably occur.
	local increment= get_speed_increment()
	local inc_large= get_speed_inc_large()
	local ret= {
		Name= "Speed",
		LayoutType= "ShowAllInRow",
		SelectType= "SelectMultiple",
		OneChoiceForAllPlayers= false,
		LoadSelections= function(self, list, pn)
			-- The first values display the current status of the speed mod.
			if pn == PLAYER_1 or self.NumPlayers == 1 then
				list[1]= true
			else
				list[2]= true
			end
		end,
		SaveSelections= function(self, list, pn)
			local val= self.CurValues[pn]
			local poptions= GAMESTATE:GetPlayerState(pn):GetPlayerOptions("ModsLevel_Preferred")
			-- modify stage, song and current too so this will work in edit mode.
			local stoptions= GAMESTATE:GetPlayerState(pn):GetPlayerOptions("ModsLevel_Stage")
			local soptions= GAMESTATE:GetPlayerState(pn):GetPlayerOptions("ModsLevel_Song")
			local coptions= GAMESTATE:GetPlayerState(pn):GetPlayerOptions("ModsLevel_Current")
			if val.mode == "x" then
				local speed= val.speed / 100
				poptions:XMod(speed)
				stoptions:XMod(speed)
				soptions:XMod(speed)
				coptions:XMod(speed)
			elseif val.mode == "C" then
				poptions:CMod(val.speed)
				stoptions:CMod(val.speed)
				soptions:CMod(val.speed)
				coptions:CMod(val.speed)
			else
				poptions:MMod(val.speed)
				stoptions:MMod(val.speed)
				soptions:MMod(val.speed)
				coptions:MMod(val.speed)
			end
		end,
		NotifyOfSelection= function(self, pn, choice)
			-- Adjust for the status elements
			local real_choice= choice - self.NumPlayers
			-- return true even though we didn't actually change anything so that
			-- the underlines will stay correct.
			if real_choice < 1 then return true end
			local val= self.CurValues[pn]
			if real_choice < 5 then
				local incs= {inc_large, increment, -increment, -inc_large}
				local new_val= val.speed + incs[real_choice]
				if new_val > 0 then
					val.speed= math.round(new_val)
				end
			elseif real_choice >= 5 then
				val.mode= ({"x", "C", "m"})[real_choice - 4]
			end
			self:GenChoices()
			MESSAGEMAN:Broadcast("SpeedChoiceChanged", {pn= pn, mode= val.mode, speed= val.speed})
			return true
		end,
		GenChoices= function(self)
			-- We can't show different options to each player, so compromise by
			-- only showing the xmod increments if one player is in that mode.
			local show_x_incs= false
			for pn, val in pairs(self.CurValues) do
				if val.mode == "x" then
					show_x_incs= true
				end
			end
			local big_inc= inc_large
			local small_inc= increment
			if show_x_incs then
				big_inc= tostring(big_inc / 100)
				small_inc= tostring(small_inc / 100)
			else
				big_inc= tostring(big_inc)
				small_inc= tostring(small_inc)
			end
			self.Choices= {
				"+" .. big_inc, "+" .. small_inc, "-" .. small_inc, "-" .. big_inc,
				"Xmod", "Cmod", "Mmod"}
			-- Insert the status element for P2 first so it will be second
			for i, pn in ipairs({PLAYER_2, PLAYER_1}) do
				local val= self.CurValues[pn]
				if val then
					if val.mode == "x" then
						table.insert(self.Choices, 1, (val.speed/100) .. "x")
					else
						table.insert(self.Choices, 1, val.mode .. val.speed)
					end
				end
			end
		end,
		CurValues= {}, -- for easy tracking of what speed the player wants
		NumPlayers= 0 -- for ease when adjusting for the status elements.
	}
	for i, pn in ipairs(GAMESTATE:GetEnabledPlayers()) do
		if GAMESTATE:IsHumanPlayer(pn) then
			local speed, mode= GetSpeedModeAndValueFromPoptions(pn)
			ret.CurValues[pn]= {mode= mode, speed= speed}
			ret.NumPlayers= ret.NumPlayers + 1
		end
	end
	ret:GenChoices()
	return ret
end

--[[
CustomSpeedMods (c) 2013 StepMania team.

Use freely, so long as this notice and the above documentation remains.

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

Previous version was copyright © 2008-2012 AJ Kelly/KKI Labs.
]]
