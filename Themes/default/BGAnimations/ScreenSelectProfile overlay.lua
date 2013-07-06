function GetLocalProfiles()
	local ret = {};

	for p = 0,PROFILEMAN:GetNumLocalProfiles()-1 do
		local profile=PROFILEMAN:GetLocalProfileFromIndex(p);
		local item = Def.ActorFrame {

			LoadFont("Common Normal") .. {
				Text=profile:GetDisplayName();
				InitCommand=function(self)
					self:shadowlength(1);
					self:y(-10);
					self:zoom(1);
					self:ztest(true);
				end;
			};
			LoadFont("Common Normal") .. {
				InitCommand=function(self)
					self:shadowlength(1);
					self:y(8);
					self:zoom(0.5);
					self:vertspacing(-8);
					self:ztest(true);
				end;
				BeginCommand=function(self)
					local numSongsPlayed = profile:GetNumTotalSongsPlayed();
					local s = numSongsPlayed == 1 and "Song" or "Songs";
					self:settext( string.format(THEME:GetString("ScreenSelectProfile","%d "..s.." Played"),numSongsPlayed) );
				end;
			};
		};
		table.insert( ret, item );
	end;

	return ret;
end;

function LoadCard(cColor)
	local t = Def.ActorFrame {
		LoadActor( THEME:GetPathG("ScreenSelectProfile","CardBackground") ) .. {
			InitCommand=function(self)
				self:diffuse(cColor);
			end;
		};
		LoadActor( THEME:GetPathG("ScreenSelectProfile","CardFrame") );
	};
	return t
end
function LoadPlayerStuff(Player)
	local ret = {};

	local pn = (Player == PLAYER_1) and 1 or 2;

	local t = Def.ActorFrame {
		Name = 'JoinFrame';
		LoadCard(Color('Orange'));

		LoadFont("Common Normal") .. {
			Text=THEME:GetString("ScreenSelectProfile","PressStart");
			InitCommand=function(self)
				self:shadowlength(1);
			end;
			OnCommand=function(self)
				self:diffuseshift();
				self:effectcolor1(Color('White'));
				self:effectcolor2(color("0.5,0.5,0.5"));
			end;
		};
	};
	table.insert( ret, t );
	
	t = Def.ActorFrame {
		Name = 'BigFrame';
		LoadCard(PlayerColor(Player));
	};
	table.insert( ret, t );

	t = Def.ActorFrame {
		Name = 'SmallFrame';

		InitCommand=function(self)
			self:y(-2);
		end;
		Def.Quad {
			InitCommand=function(self)
				self:zoomto(200-10, 40+2);
			end;
			OnCommand=function(self)
				self:diffuse(Color('Black'));
				self:diffusealpha(0.5);
			end;
		};
		Def.Quad {
			InitCommand=function(self)
				self:zoomto(200-10, 40);
			end;
			OnCommand=function(self)
				self:diffuse(PlayerColor(Player));
				self:fadeleft(0.25);
				self:faderight(0.25);
				self:glow(color("1,1,1,0.25"));
			end;
		};
		Def.Quad {
			InitCommand=function(self)
				self:zoomto(200-10, 40);
				self:y(-40 / 2 + 20);
			end;
			OnCommand=function(self)
				self:diffuse(Color("Black"));
				self:fadebottom(1);
				self:diffusealpha(0.35);
			end;
		};
		Def.Quad {
			InitCommand=function(self)
				self:zoomto(200 - 10, 1);
				self:y(-40 / 2 + 1);
			end;
			OnCommand=function(self)
				self:diffuse(PlayerColor(Player));
				self:glow(color("1,1,1,0.25"));
			end;
		};	
	};
	table.insert( ret, t );

	t = Def.ActorScroller{
		Name = 'ProfileScroller';
		NumItemsToDraw=6;
		OnCommand=function(self)
			self:y(1);
			self:SetFastCatchup(true);
			self:SetMask(200, 58);
			self:SetSecondsPerItem(0.15);
		end;
		TransformFunction=function(self, offset, itemIndex, numItems)
			local focus = scale(math.abs(offset),0,2,1,0);
			self:visible(false);
			self:y(math.floor( offset*40 ));
		end;
		children = GetLocalProfiles();
	};
	table.insert( ret, t );
	
	t = Def.ActorFrame {
		Name = "EffectFrame";
	};
	table.insert( ret, t );
	t = LoadFont("Common Normal") .. {
		Name = 'SelectedProfileText';
		InitCommand=function(self)
			self:y(160);
			self:shadowlength(1);
		end;
	};
	table.insert( ret, t );

	return ret;
end;

function UpdateInternal3(self, Player)
	local pn = (Player == PLAYER_1) and 1 or 2;
	local frame = self:GetChild(string.format('P%uFrame', pn));
	local scroller = frame:GetChild('ProfileScroller');
	local seltext = frame:GetChild('SelectedProfileText');
	local joinframe = frame:GetChild('JoinFrame');
	local smallframe = frame:GetChild('SmallFrame');
	local bigframe = frame:GetChild('BigFrame');

	if GAMESTATE:IsHumanPlayer(Player) then
		frame:visible(true);
		if MEMCARDMAN:GetCardState(Player) == 'MemoryCardState_none' then
			--using profile if any
			joinframe:visible(false);
			smallframe:visible(true);
			bigframe:visible(true);
			seltext:visible(true);
			scroller:visible(true);
			local ind = SCREENMAN:GetTopScreen():GetProfileIndex(Player);
			if ind > 0 then
				scroller:SetDestinationItem(ind-1);
				seltext:settext(PROFILEMAN:GetLocalProfileFromIndex(ind-1):GetDisplayName());
			else
				if SCREENMAN:GetTopScreen():SetProfileIndex(Player, 1) then
					scroller:SetDestinationItem(0);
					self:queuecommand('UpdateInternal2');
				else
					joinframe:visible(true);
					smallframe:visible(false);
					bigframe:visible(false);
					scroller:visible(false);
					seltext:settext('No profile');
				end;
			end;
		else
			--using card
			smallframe:visible(false);
			scroller:visible(false);
			seltext:settext('CARD');
			SCREENMAN:GetTopScreen():SetProfileIndex(Player, 0);
		end;
	else
		joinframe:visible(true);
		scroller:visible(false);
		seltext:visible(false);
		smallframe:visible(false);
		bigframe:visible(false);
	end;
end;

local t = Def.ActorFrame {

	StorageDevicesChangedMessageCommand=function(self, params)
		self:queuecommand('UpdateInternal2');
	end;

	CodeMessageCommand = function(self, params)
		if params.Name == 'Start' or params.Name == 'Center' then
			MESSAGEMAN:Broadcast("StartButton");
			if not GAMESTATE:IsHumanPlayer(params.PlayerNumber) then
				SCREENMAN:GetTopScreen():SetProfileIndex(params.PlayerNumber, -1);
			else
				SCREENMAN:GetTopScreen():Finish();
			end;
		end;
		if params.Name == 'Up' or params.Name == 'Up2' or params.Name == 'DownLeft' then
			if GAMESTATE:IsHumanPlayer(params.PlayerNumber) then
				local ind = SCREENMAN:GetTopScreen():GetProfileIndex(params.PlayerNumber);
				if ind > 1 then
					if SCREENMAN:GetTopScreen():SetProfileIndex(params.PlayerNumber, ind - 1 ) then
						MESSAGEMAN:Broadcast("DirectionButton");
						self:queuecommand('UpdateInternal2');
					end;
				end;
			end;
		end;
		if params.Name == 'Down' or params.Name == 'Down2' or params.Name == 'DownRight' then
			if GAMESTATE:IsHumanPlayer(params.PlayerNumber) then
				local ind = SCREENMAN:GetTopScreen():GetProfileIndex(params.PlayerNumber);
				if ind > 0 then
					if SCREENMAN:GetTopScreen():SetProfileIndex(params.PlayerNumber, ind + 1 ) then
						MESSAGEMAN:Broadcast("DirectionButton");
						self:queuecommand('UpdateInternal2');
					end;
				end;
			end;
		end;
		if params.Name == 'Back' then
			if GAMESTATE:GetNumPlayersEnabled()==0 then
				SCREENMAN:GetTopScreen():Cancel();
			else
				MESSAGEMAN:Broadcast("BackButton");
				SCREENMAN:GetTopScreen():SetProfileIndex(params.PlayerNumber, -2);
			end;
		end;
	end;

	PlayerJoinedMessageCommand=function(self, params)
		self:queuecommand('UpdateInternal2');
	end;

	PlayerUnjoinedMessageCommand=function(self, params)
		self:queuecommand('UpdateInternal2');
	end;

	OnCommand=function(self, params)
		self:queuecommand('UpdateInternal2');
	end;

	UpdateInternal2Command=function(self)
		UpdateInternal3(self, PLAYER_1);
		UpdateInternal3(self, PLAYER_2);
	end;

	children = {
		Def.ActorFrame {
			Name = 'P1Frame';
			InitCommand=function(self)
				self:x(SCREEN_CENTER_X - 160);
				self:y(SCREEN_CENTER_Y);
			end;
			OnCommand=function(self)
				self:zoom(0);
				self:bounceend(0.35);
				self:zoom(1);
			end;
			OffCommand=function(self)
				self:bouncebegin(0.35);
				self:zoom(0);
			end;
			PlayerJoinedMessageCommand=function(self,param)
				if param.Player == PLAYER_1 then
					self:zoom(1.15);
					self:bounceend(0.175);
					self:zoom(1.0);
				end;
			end;
			children = LoadPlayerStuff(PLAYER_1);
		};
		Def.ActorFrame {
			Name = 'P2Frame';
			InitCommand=function(self)
				self:x(SCREEN_CENTER_X + 160);
				self:y(SCREEN_CENTER_Y);
			end;
			OnCommand=function(self)
				self:zoom(0);
				self:bounceend(0.35);
				self:zoom(1);
			end;
			OffCommand=function(self)
				self:bouncebegin(0.35);
				self:zoom(0);
			end;
			PlayerJoinedMessageCommand=function(self,param)
				if param.Player == PLAYER_2 then
					self:zoom(1.15);
					self:bounceend(0.175);
					self:zoom(1.0);
				end;
			end;
			children = LoadPlayerStuff(PLAYER_2);
		};
		-- sounds
		LoadActor( THEME:GetPathS("Common","start") )..{
			StartButtonMessageCommand=function(self)
				self:play();
			end;
		};
		LoadActor( THEME:GetPathS("Common","cancel") )..{
			BackButtonMessageCommand=function(self)
				self:play();
			end;
		};
		LoadActor( THEME:GetPathS("Common","value") )..{
			DirectionButtonMessageCommand=function(self)
				self:play();
			end;
		};
	};
};

return t;
