local PlayerIndexes = { PlayerNumber_P1 = 1, PlayerNumber_P2 = 1 }
-- Reads mods from a file
-- keywords: Speed, Noteskins, None, Clear
local function ReadMods()
	local file = THEME:GetCurrentThemeDirectory() .. "mods.txt"
	local mods = File.Read( file )
	if not mods
	or mods == ""
	then
		local buff = ";Set mods here, one per line or various in one separated with commas\r\n;Keywords: 'Speed', 'Noteskins', 'None', 'Clear'\r\nSpeeds\r\nNoteskins"
		File.Write(file,buff)
		return {
			ReadSpeedMods();
			NOTESKIN:GetNoteSkinNames();
			"Clear";
		}
	end
	
	mods = string.gsub(mods,"\r","")
	mods = split("\n",mods)
	
	local ret = {}
	local clear = false
	
	for k,v in ipairs(mods) do
		if string.sub(v,1,1) == ";"
		or v == ""
		then
			--skip
		elseif v == "Speeds" then
			table.insert(ret, ReadSpeedMods())
		elseif v == "Noteskins" then
			table.insert(ret, NOTESKIN:GetNoteSkinNames())
		else
			local ModToAdd = string.gsub(v,"%s","")
			if string.find(ModToAdd, ",") ~= nil then
				ModToAdd = split(",", ModToAdd)
				if ModToAdd[1] ~= "None" then
					table.insert(ModToAdd,1,"None")
				end
				table.insert(ret,ModToAdd)
			else
				if ModToAdd == "Clear" then
					if not clear then table.insert(ret,ModToAdd) end
					clear = true
				end
			end
		end
	end
	-- This will happen...
	if not clear then table.insert(ret,"Clear") end
	
	return ret
end

local modslist = ReadMods()

-- Displays the mods with actorscrollers
local function OptionsListDisplay(player)
	local function GetPlayerMods()
		local playerstate = GAMESTATE:GetPlayerState(player)
		local playermods = playerstate:GetPlayerOptionsArray('ModsLevel_Preferred')
		return playermods
	end
	
	local make = {}
	-- iterate modslist entries
	for k,v in ipairs(modslist) do
		-- not kinopio
		local RowToAdd
		if type(v) == "table" then
			-- it's a table, make it a scroller
			local choices = {}
			-- build the choices
			for i,j in ipairs(v) do
				choices[i] = LoadFont()..{ Text=j };
			end
			RowToAdd = Def.ActorScroller {
				NumItemsToDraw=3;
				SecondsPerItem=0.075;
				-- init indexes here
				-- I like how InitCommand is not allowed to be played or queued
				-- (HURR TOO MUCH COMMANDS ARG!)
				SetChoicesCommand=function(self)
					local found = false
					--SCREENMAN:SystemMessage(join(",", defmods))
					for idx,Mod in ipairs(GetPlayerMods()) do
						-- In before I forgot what's this: those are the nod entries
						for jdx,Nod in ipairs(v) do
							if Mod == Nod then
								found = true
								self:SetCurrentAndDestinationItem(jdx-1);
								break
							end
						end
					end
					-- oops, no defaults, set them
					if not found then
						for idx,Mod in ipairs(v) do
							if Mod == "1x"
							or Mod == "default"
							or Mod == "None"
							then
								self:SetCurrentAndDestinationItem(idx-1);
								break
							else
								self:SetCurrentAndDestinationItem(0);
							end
						end
					end
				end;
				CleanModsMessageCommand=function(self,params)
					if player ~= params.PlayerNumber then return end
					self:playcommand("SetChoices");
				end;
				InitCommand=function(self)
					self:SetLoop(true);
					self:playcommand("SetChoices");
				end;
				TransformFunction=function(self, offset, itemIndex, numItems)
					self:x(offset * 100);
					local focus = math.abs(offset)
					focus = scale(focus,0,1,1,0.75)
					
					self:zoom(focus)
				end;
				ChoiceSwapMessageCommand=function(self,params)
					if params.PlayerNumber ~= player then return end
					if params.Row ~= k then return end
					
					local index = params.Index
					--SCREENMAN:SystemMessage(ToEnumShortString(player).." row mods index: "..index);
					if params.bHasLooped then
						self:SetCurrentAndDestinationItem(-1)
					end
					
					self:SetDestinationItem(index-1)
				end;
				-- actors are wrapped here:
				children = choices;
			};
		elseif type(v) == "string" then
			-- it's not, make single switchable choice
			RowToAdd = Def.ActorFrame {
				LoadFont()..{
					Text=v;
					-- init highlights here
					SetChoicesCommand=function(self)
						local cc = color("#000000")
						for idx,Mod in ipairs(GetPlayerMods()) do
							if Mod == v then
								cc = color("#ffffff")
								break
							end
						end
						if v == "Clear" then
							cc = color("#0000ff")
						end
						self:diffuse(cc);
					end;
					InitCommand=function(self)
						self:playcommand("SetChoices");
					end;
					-- Following commands you'll understand (I've heard that somewhere...)
					CleanModsMessageCommand=function(self,params)
						--if v == "Clear" then return end
						if player ~= params.PlayerNumber then return end
						self:playcommand("SetChoices");
						--self:diffuse(color("#000000"));
					end;
					-- cmd(diffuse,color("#000000"));
					ChoiceToggleMessageCommand=function(self,params)
						if params.PlayerNumber ~= player then return end
						if params.Row ~= k then return end
						
						local cc = color("#ffffff")
						if not params.Activated then
							cc = color("#000000")
						end
						
						--self:visible(params.Activated)
						-- why the fuck is not coloring itself? oh I think I got it... duh!
						self:diffuse(cc)
					end;
				}
			}
		else
			--table.remove(modslist,k)
			--Def.Actor { };
		end
		make[k] = RowToAdd
	end
	
	return Def.ActorFrame {
		Draw.RoundBox(300,320,50,50,color("1,0,1,0.5"));
		Draw.RoundBox(300,50,25,25,color("1,0,0,0.5"));
		Def.ActorScroller {
			Name="OptionsListScroller";
			NumItemsToDraw=5;
			SecondsPerItem=0.075;
			InitCommand=function(self)
				self:SetLoop(true);
			end;
			TransformFunction=function(self, offset, itemIndex, numItems)
				self:y(offset * 50);
			end;
			children = make;
		};
	}
end

-- Internals
return Def.ActorFrame {
	OptionsListDisplay(PLAYER_1)..{
		Name="OptionsListDisplay"..PLAYER_1;
		Condition=GAMESTATE:IsHumanPlayer(PLAYER_1);
		InitCommand=function(self)
			if #GAMESTATE:GetHumanPlayers() == 1
			and PREFSMAN:GetPreference("Center1Player")
			then
				self:CenterX()
			else
				self:FromLeft(160)
			end
			self:CenterY();
		end;
	};
	OptionsListDisplay(PLAYER_2)..{
		Name="OptionsListDisplay"..PLAYER_2;
		Condition=GAMESTATE:IsHumanPlayer(PLAYER_2);
		InitCommand=function(self)
			if #GAMESTATE:GetHumanPlayers() == 1
			and PREFSMAN:GetPreference("Center1Player")
			then
				self:CenterX()
			else
				self:FromRight(-160)
			end
			self:CenterY();
		end;
	};
	-- Everything here and beyond is pure magic
	CodeMessageCommand=function(self,params)
		-- I'm reusing code from CustomCodeDetector
		local code = params.Name
		local player = params.PlayerNumber
		local movement = "none"
		local modscleared = false
		
		if not GAMESTATE:IsHumanPlayer(player) then return end;
		
		if code == "Back" then
			--SCREENMAN:SystemMessage(ToEnumShortString(player).." closed the oplist")
			SCREENMAN:GetTopScreen():Cancel();
		elseif code == "Prev" then
			PlayerIndexes[player] = PlayerIndexes[player]-1
			if PlayerIndexes[player] < 1 then
				PlayerIndexes[player] = #modslist
				movement = "first"
			end
			--SCREENMAN:SystemMessage(ToEnumShortString(player).." index: "..PlayerIndexes[player]);
		elseif code == "Next" then
			PlayerIndexes[player] = PlayerIndexes[player]+1
			if PlayerIndexes[player] > #modslist then
				PlayerIndexes[player] = 1
				movement = "last"
			end
			--SCREENMAN:SystemMessage(ToEnumShortString(player).." index: "..PlayerIndexes[player]);
		elseif code == "Toggle" then
			-- changes will be done here
			-- get playerstate and playermods
			local playerstate = GAMESTATE:GetPlayerState(player)
			local playermods = playerstate:GetPlayerOptionsArray('ModsLevel_Preferred')
			
			-- let's check in what index we are
			local ModToAdd = modslist[PlayerIndexes[player]]
			--the mod is a table, scroll those
			if type(ModToAdd) == "table" then
				-- iteration, once again, like in the custom code detector I made :x
				local index = 1
				local found = false
				local looped = false
				for i,j in ipairs(playermods) do
					for k,l in ipairs(ModToAdd) do
						if j == l then
							found = true
							index = k+1
							if index > #ModToAdd then
								index = 1
								looped = true
							end
							--replace
							playermods[i] = ModToAdd[index]
							break
						end
					end
				end
				-- looks like nothing was found, we can assume those are the default mods
				-- (1x and default noteskin) they are not listed for some reason...
				if not found then
					-- set the first on the list (nono, better list 1x and default)
					for k,v in ipairs(ModToAdd) do
						if v == "1x"
						or v == "default"
						then
							index = k+1
							if index > #ModToAdd then
								index = 1
								looped = true
							end
							table.insert(playermods, ModToAdd[index])
							break
						elseif v == "None" then
							table.insert(playermods, ModToAdd[2])
							index = 2
						end
					end
				end
				-- send message broadcasts to be able to move things on the oplist displays
				MESSAGEMAN:Broadcast("ChoiceSwap",{ Row = PlayerIndexes[player]; Index = index; PlayerNumber = player; bHasLooped = looped })
			elseif ModToAdd == "Clear" then
				local defaultmods = PREFSMAN:GetPreference("DefaultModifiers")
				playermods = split(",", defaultmods)
				modscleared = true
				--MESSAGEMAN:Broadcast("CleanMods",{ PlayerNumber = player });
			else
				-- if type(ModToAdd) == "string" then end
				-- we know it's a string...
				local found = false
				for i,j in ipairs(playermods) do
					if j == ModToAdd then
						-- remove it
						found = true
						table.remove(playermods, i)
						break
					end
				end
				
				if not found then
					table.insert(playermods, ModToAdd)
				end
				MESSAGEMAN:Broadcast("ChoiceToggle",{ Row = PlayerIndexes[player]; Activated = not found; PlayerNumber = player })
			end
			-- let's check if noteskins aren't missing (we're avoiding crashes here...)
			local found = false
			-- iterate again uh
			for i,j in ipairs(playermods) do
				-- yes because noteskins are stored in the second slot...
				for k,l in ipairs(modslist[2]) do
					if j == l then
						found = true
						break
					end
				end
			end
			-- apparently there aren't any noteskins...
			if not found then
				table.insert(playermods, "default")
			end
			
			-- everything is ready to be set
			playermods = join(", ",playermods)
			playerstate:SetPlayerOptions('ModsLevel_Preferred',playermods)
			-- and thats pretty much it :D
		end
		
		if modscleared then
			MESSAGEMAN:Broadcast("CleanMods",{ PlayerNumber = player });
		end
		
		--this moves the scroller...
		local scroller = self:GetChild("OptionsListDisplay"..player):GetChild("OptionsListScroller")
		if movement ~= "none" then
			local tomove
			if movement == "first" then
				tomove = PlayerIndexes[player]
			elseif movement == "last" then
				tomove = PlayerIndexes[player]-2
			end
			scroller:SetCurrentAndDestinationItem(tomove);
		end
		scroller:SetDestinationItem(PlayerIndexes[player]-1);
	end;
};