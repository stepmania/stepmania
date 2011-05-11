-- CH32 asked me if this was possible, and as you can see, it is!!
local function GetSpeedChanges()
	local smfile = GAMESTATE:GetCurrentSong():GetSongFilePath()
	local file = File.Read( smfile )
	if not file then return end
	-- search the tag
	local find = string.find(file , "#SPEED:")
	if not find then return end
	-- search the next semicolon
	local last = string.find(file , ";" , find)
	local found = string.sub(file,find,last)
	-- cleans
	found = string.gsub(found, "\r", "")
	found = string.gsub(found, "\n", "")
	found = string.gsub(found, "#SPEED:", "")
	found = string.gsub(found, ";", "")
	-- make it a table
	found = split(",", found)
	-- return it
	return found
end

local speeds = GetSpeedChanges()

-- what are those things made of?
local function update(self)
	if speeds ~= nil then
		for idx,Spd in ipairs(speeds) do
			local part = split("=", Spd)
			local ebeat = tonumber(part[1]) --WHERE
			assert(ebeat)
			local songbeat = GAMESTATE:GetSongBeat()
			
			if songbeat >= ebeat
			-- 1/20 beat
			and songbeat <= ebeat + (1/20)
			--and not GAMESTATE:GetSongFreeze()
			--and not GAMESTATE:GetSongDelay()
			then
				local bps = 60 / GAMESTATE:GetCurrentSong():GetTimingData():GetBPMAtBeat( ebeat )
				--local bps = GAMESTATE:GetSongBPS()
				local value = tonumber(part[2]) --SPEED
				local approach = part[3] --APPLY TIME
				-- Odd, it needs to be a string to be evaluated o_o;
				if not approach then approach = tostring(4*bps) end
				
				if string.sub(approach,approach:len()) == "s" then
					approach = string.sub(approach,1,approach:len()-1)
					approach = tonumber(approach)
					approach = 1/approach
				else
					approach = 1/(approach*bps)
				end
				
				local pastspeed = 1
				if idx > 1 then
					pastspeed = split("=", speeds[idx-1])
					pastspeed = tonumber(pastspeed[2])
				end
				
				approach = approach * math.abs(value-pastspeed)
				
				approach = approach + 0.05
				
				--if value ~= 0 then
					--approach = value/approach
					--approach = scale(value,0,approach,0,1)
				--end
				
				MESSAGEMAN:Broadcast("SpeedModLaunched",{ Value = value ; Approach = approach });
				--SCREENMAN:SystemMessage("APP: " .. approach .. " SPD: " .. value);
				return
			end
		end
	end
end

return Def.ActorFrame {
	InitCommand=cmd(SetUpdateFunction,update);
	SpeedModLaunchedMessageCommand=function(self,params)
		local approach = params.Approach
		local value = params.Value
		
		local playerstate = GAMESTATE:GetPlayerState(PLAYER_1)
		local playermods = playerstate:GetPlayerOptionsArray('ModsLevel_Preferred')
		
		local found = false
		for idx,Mod in ipairs(playermods) do
			if string.find(Mod,"%dx") ~= nil
			--or string.find(Mod,"%d.%dx") ~= nil
			then
				found = true
				local xmod = playermods[idx]
				xmod = string.gsub(xmod,"x","")
				xmod = tonumber(xmod)
				xmod = xmod * value
				--duh, floats
				xmod = string.format("*%.2f %.2fx",approach,xmod)
				playermods[idx] = xmod
				--SCREENMAN:SystemMessage(xmod)
			end
		end
		if not found then
			local xmod = string.format("*%.2f %.2fx",approach,value)
			table.insert( playermods, xmod)
		end
		
		playermods = join(",",playermods)
		playerstate:SetPlayerOptions('ModsLevel_Song', playermods)
		--self:GetChild("_debug"):settext(playermods)
		--SCREENMAN:SystemMessage(playerstate:GetPlayerOptions('ModsLevel_Preferred'));
		--SCREENMAN:SystemMessage(value)
	end;
	--LoadFont()..{ Name="_debug"; InitCommand=cmd(x,50;CenterY;rotationz,90); };
}