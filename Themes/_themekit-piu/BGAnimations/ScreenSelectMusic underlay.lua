return Def.ActorFrame {
	InitCommand=function(self)
		setenv("GradeReverse" .. PLAYER_1, false)
		setenv("GradeReverse" .. PLAYER_2, false)
		--setenv("UnderAttack", false)
		--setenv("Drop", false)
	end;
	CodeMessageCommand=function(self,params)
		local code = params.Name
		local player = params.PlayerNumber
		if not GAMESTATE:IsHumanPlayer(player) then return end;
		
		if not GetUserPrefB("OptionsMode") then
			--Yeah
			if not CustomCodeDetector(code,player,true) then
				--handling envutils here
				if code == "GradeReverse"
				--or code == "any envutil wich requires to be per player"
				then
					if not getenv( code..player ) then
						setenv( code..player, true)
					else
						setenv( code..player, false)
					end
				else --general envutils (not per player)
					if not getenv( code ) then
						setenv( code, true)
					else
						setenv( code, false)
					end
				end
			end
			
			if code == "Cancel" then
				setenv("GradeReverse"..player,false)
			end
			
			MESSAGEMAN:Broadcast("PlayerOptionsChanged", { PlayerNumber = player })
			--SOUND:PlayOnce(THEME:GetPathS("ScreenSelectMusic","Options"))
		else
			if code == "Summon" then
				--this lua binding is awesome
				--SCREENMAN:SystemMessage(ToEnumShortString(player).." opened the oplist")
				self:sleep(0.5);
				self:queuecommand("OpenCustomOpList");
			end
		end
	end;
	OpenCustomOpListCommand=function(self) SCREENMAN:AddNewScreenToTop("ScreenCustomOptionsList") end;
	PlayerOptionsChangedMessageCommand=function(self,params)
		local player = params.PlayerNumber
		local playermods = GAMESTATE:GetPlayerState(player):GetPlayerOptions('ModsLevel_Preferred')
		--SCREENMAN:SystemMessage(ToEnumShortString(player).." mods: "..playermods);
	end;
	LoadSound("START")..{
		OffCommand=cmd(play);
	};
	LoadSound("ScreenSelectMusic","Options")..{
		PlayerOptionsChangedMessageCommand=cmd(stop;play);
	};
	LoadSound("3-2")..{
		--TwoPartConfirmCanceledMessageCommand=cmd(play);
		SongChosenMessageCommand=cmd(stop;play);
		OffCommand=cmd(play);
	};
}
