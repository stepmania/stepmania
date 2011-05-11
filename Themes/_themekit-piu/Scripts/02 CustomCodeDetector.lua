--moved here for portability sake
--[[--------------------------------------------------------------------------
Custom Code Detector by Daisuke Master
------------------------------------------------------------------------------
-= Whats this? =-
Just that, a custom codedetector, you can add the codenames as exact mods
and this thingy will do whatever you wanted to do with the oh-so-known
harcoded codedetector.
I kind of don't like the codedetector for various reasons, that's why I made
this (also the remaining stuff happened by accident lol), anyway, if you feel
like wanting this in your theme, go ahead! no problem, just let me know that
you're using it and that this was useful for you ^^

-= How to use =-
First of all, put this lua module under the Scripts folder of your theme.
Using codeset, put this in a CodeMessageCommand, inside a function command,
under ScreenSelectMusic, like this:

<code>
CodeMessageCommand=function(self)
	CustomCodeDetector()
end;
</code>

You'll need codename and player to be passed as arguments, you can get those
somewhere:

<code>
CodeMessageCommand=function(self,params)
	CustomCodeDetector(params.Name, params.PlayerNumber)
end;
</code>

If you like to be classy like me:

<code>
CodeMessageCommand=function(self,params)
	local code = params.Name
	local player = params.PlayerNumber
	if not GAMESTATE:IsHumanPlayer(player) then return end;
	
	CustomCodeDetector(code, player)
end;
</code>

Niceties:

<code>
CodeMessageCommand=function(self,params)
	local code = params.Name
	local player = params.PlayerNumber
	if not GAMESTATE:IsHumanPlayer(player) then return end;
	
	if CustomCodeDetector(code, player, true) then
		--idk what to put here so eh...
	else
		--handle here other codes for misc purposes
	end
	
	SOUND:PlayOnce(THEME:GetPathS("ScreenSelectMusic","Options"))
end;
</code>
Extras: you can set a third param as true to deactivate sound and
the message broadcast to handle those outside (example above)

What more? Oh yes, you'll need a CodeNames line under ScreenSelectMusic

<code>
[ScreenSelectMusic]
CodeNames="Noteskin,Speed,Cancel,Dark,Mirror"
;main codes
CodeNoteskin="Up,Up,Down,Down,Left,Right,Left,Right"
CodeSpeed="Left,Right,Left,Right,Down"
CodeCancel="Select,Select"
;secondary codes
CodeDark="Down,Down,Left,Right,Down"
CodeMirror="Down,Left,Up,Down,Right,Up"
</code>

The first three (speed, noteskin and cancle er... cancel) are mandatory, the
rest are optional (dark and mirror as examples)

-= Caveats =-
*I don't know what will happen if illegal modifiers are set into the CodeNames
line under ScreenSelectMusic, so try at your own risk (not that anything bad
but a crash may happen, I don't know so I'm just warning you).
*I don't know what will happen if you add percentages and/or approach into the
modifiers (for example >Code50% tornado="@Right-Up")
*Ignore the previous two points, I just added a mod validator function, if your
mod does not exists there just add it.
*You just can't set multiple modifiers in one code because of how codes are
separated (by commas)
*FOR PUMP-SM USERS: Watch out when using center as part of your code, it's
treated as start button and you know what happens when you press start,
solution: you can use two part selection in metrics along with twopart
confirms only so you can doublepress center with no problems, remember that
only sm-ssc can do that (goes the same to ez2/popn users with footdown/red
buttons).
*Don't try weird things with this.

-= TODO list =-
*Previous speed and noteskins
*Go lazy and lowercase GetPlayerOptions
*Be able to put multiple mods in one code

-= Licensing =-
IMO it's important, I don't like to see my work being hijacked by someone else
and claiming as it's own (others don't like that either) so here we go:

I'm licensing this under Creative Commons by-sa 3.0
http://creativecommons.org/licenses/by-sa/3.0/

This means that:

You can share and adapt this code as you like/need/want withouth worry, but
you must attribute me, and if you release your modified work you must license
it with the same license as this.

-= Credits =-
ºDaisuMaster

-= Thanks =-
ºmain stepmania devteam (they created SM)
ºsm-ssc devteam (they forked sm4 and tweaked it in an astounding way)
ºcesarmades (this guy inspired me somehow)

-= Final notes =-
That's all you should need to know for now, and as I said, feel free to use
this and let me know if this was useful for you ^^
--]]--------------------------------------------------------------------------
function ReadSpeedMods()
	--this reads speeds from a file (Cmods are not allowed, I don't like to
	--deal with these)
	local dir = THEME:GetCurrentThemeDirectory() .. "speeds.txt"
	local speeds = File.Read( dir )
	if not speeds then
		speeds = {"1x","2x","3x","4x","5x","8x"}
		speeds = join("\r\n",speeds)
		File.Write( dir , speeds)
		return speeds
	end
	
	speeds = string.gsub(speeds,"\r","")
	speeds = split("\n", speeds)
	
	--[[local rebuild = false
	for k,v in ipairs(speeds) do
		if not string.find(v,"%dx") or not string.find(v,"%%d.dx") then
			rebuild = true
			table.remove(speed,k)
		end
	end
	if rebuild then
		--cleanup
		File.Write( dir , join("\r\n",speeds))
	end]]
	
	return speeds
end

local function IsValidMod(m)
	--check mods here, see PlayerOptions.cpp if you want/need more
	--throwing the most common/used here
	--todo: regex formats and bind the mods into a table so setups like 50% dark would work
	if m == "Boost"
	or m == "Brake"
	or m == "Wave"
	or m == "Expand"
	or m == "Boomerang"
	or m == "Drunk"
	or m == "Dizzy"
	or m == "Confusion"
	or m == "Mini"
	or m == "Tiny"
	or m == "Flip"
	or m == "Invert"
	or m == "Tornado"
	or m == "Tipsy"
	or m == "Bumpy"
	or m == "Beat"
	or m == "XMode"
	or m == "Hidden"
	or m == "Sudden"
	or m == "Stealth"
	or m == "Blink"
	or m == "RandomVanish"
	or m == "Reverse"
	or m == "Split"
	or m == "Alternate"
	or m == "Cross"
	or m == "Centered"
	or m == "Dark"
	or m == "RandomAttacks"
	or m == "SongAttacks"
	or m == "PlayerAutoPlay"
	or m == "Mirror"
	or m == "Left"
	or m == "Right"
	or m == "Shuffle"
	or m == "SoftShuffle"
	or m == "SuperShuffle"
	then
		return true
	else
		return false
	end
end

--local function Tracef(...) Trace(string.format(...)) end

function CustomCodeDetector(code,player,sound_and_broadcast)
	assert(code,"[CUSTOMCODEDETECTOR]: an input code must be given.")
	assert(player, "[CUSTOMCODEDETECTOR]: a player number must be given.")
	if sound_and_broadcast then
		Trace("[CUSTOMCODEDETECTOR]: playeroptions message broadcast and sound playback are deactivated")
	end
	--playerstates lets you get and set modifiers per player via lua
	local playerstate = GAMESTATE:GetPlayerState(player)
	--nice, I didn't knew that playermods could be get as an array.
	local playermods = playerstate:GetPlayerOptionsArray('ModsLevel_Preferred')
	--local playermods = playerstate:GetPlayerOptions('ModsLevel_Preferred')
	--playermods = split(", ",playermods)
	
	local noteskins = NOTESKIN:GetNoteSkinNames()
	if not noteskins then noteskins = {"default"} end
	--woot! noteskins via codes!! and not hardcoded!!! :awesome:
	--dang, I forgot that GetNoteSkinNames works only in sm-ssc
	if code == "Noteskin" and SSC then
		
		--do changes here
		local found = false
		--iterate every noteskin in every playermod
		for i,j in ipairs(playermods) do
			for k,l in ipairs(noteskins) do
				--we got a match
				if j == l then
					found = true
					local index = k+1
					--avoiding overflow
					if index > #noteskins then index = 1 end
					--replacing
					Tracef("[CUSTOMCODEDETECTOR]: Setting noteskin '%s' in place of '%s'", noteskins[index], playermods[i])
					playermods[i] = noteskins[index]
				end
			end
		end
		
		--no matches
		if not found then
			for k,v in ipairs(noteskins) do
				if string.lower(v) == "default" then
					--set the one next to default or the first if it's the last
					local idx = k+1
					if idx > #noteskins then idx = 1 end
					Tracef("[CUSTOMCODEDETECTOR]: Setting noteskin '%s'", noteskins[idx])
					table.insert(playermods, noteskins[idx])
				end
			end
		end
	--like notekins but with speeds
	elseif code == "Speed" then
		--read your speeds from a file (by theme)
		--local speeds = { "1x","2x","3x","4x","5x" }
		local speeds = ReadSpeedMods()
		local found = false
		
		for i,j in ipairs(playermods) do
			for k,l in ipairs(speeds) do
				if j == l then
					found = true
					local index = k+1
					if index > #speeds then index = 1 end
					Tracef("[CUSTOMCODEDETECTOR]: Setting speedmod '%s' in place of '%s'", speeds[index], playermods[i])
					playermods[i] = speeds[index]
				end
			end
		end
		if not found then
			--most likely is a 1x, set the next
			for k,v in ipairs(speeds) do
				if v == "1x" then
					local idx = k+1
					if idx > #speeds then idx = 1 end
					Tracef("[CUSTOMCODEDETECTOR]: Setting speedmod '%s'", speeds[idx])
					table.insert(playermods, speeds[idx])
				end
			end
		end
	elseif code == "Cancel" then
		local defaultmods = PREFSMAN:GetPreference("DefaultModifiers")
		defaultmods = split(",", defaultmods)
		--set 1x and clear other mods, also default noteskin (along with the default mods)
		playermods = { "1x","default" }
		for k,v in ipairs(defaultmods) do
			table.insert(playermods, v)
		end
	else
		if IsValidMod(code) then
			--other mod codes, code name must be the exact mod
			local found = false
			for i,j in ipairs(playermods) do
				if j == code then
					--remove it
					found = true
					Tracef("[CUSTOMCODEDETECTOR]: Setting modifier '%s'", code)
					table.remove(playermods, i)
				end
			end
			
			if not found then
				Tracef("[CUSTOMCODEDETECTOR]: Unsetting modifier '%s'", code)
				table.insert(playermods, code)
			end
		else
			--SOUND:PlayOnce(THEME:GetPathS("MusicWheel","locked"))
			Tracef("[CUSTOMCODEDETECTOR]: The mod '%s' could not be set", code)
			return false
		end
	end
	
	--let's check if noteskins aren't missing (we're avoiding crashes here...)
	local found = false
	--iterate again uh
	for i,j in ipairs(playermods) do
		for k,l in ipairs(noteskins) do
			if j == l then
				found = true
			end
		end
	end
	--apparently there aren't any noteskins...
	if not found then
		table.insert(playermods, "default")
	end
	
	--join
	playermods = join(", ",playermods)
	
	--set the mods
	playerstate:SetPlayerOptions('ModsLevel_Preferred',playermods)
	--optional stuff
	if not sound_and_broadcast then
		--play some sound
		SOUND:PlayOnce(THEME:GetPathS("ScreenSelectMusic","Options"))
		--send some message
		MESSAGEMAN:Broadcast("PlayerOptionsChanged", { PlayerNumber = player })
	end
	--if we get here then everything went just fine
	return true
end