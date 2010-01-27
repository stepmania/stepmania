function GetLocalProfiles()
	local ret = {};

	for p = 0,PROFILEMAN:GetNumLocalProfiles()-1 do
		local profile=PROFILEMAN:GetLocalProfileFromIndex(p);
		local item = Def.ActorFrame {
			LoadFont("Common Normal") .. {
				Text=profile:GetDisplayName();
				InitCommand=cmd(shadowlength,1;y,-9);
			};
			LoadFont("Common Normal") .. {
				Text=profile:GetNumTotalSongsPlayed() .. " Songs Played";
				InitCommand=cmd(shadowlength,1;y,8;zoom,0.5;vertspacing,-8);
			};
		};
		table.insert( ret, item );
	end;	
	return ret;
end;

function LoadPlayerStuff(Player)
	local ret = {};

	local pn;
	if Player == PLAYER_1 then
		pn = 1;
	else
		pn = 2;
	end;

--[[ 	local t = LoadActor(THEME:GetPathB('', '_frame 3x3'), 'metal', 200, 230) .. {
		Name = 'BigFrame';
	}; --]]
	local t = Def.ActorFrame {
		Name = 'BigFrame';
		Def.Quad {
			InitCommand=cmd(zoomto,200+4,230+4);
			OnCommand=cmd(shadowlength,1);
		};
		Def.Quad {
			InitCommand=cmd(zoomto,200,230);
			OnCommand=cmd(diffuse,color("0.25,0,0.15,1"));
		};
	};
	table.insert( ret, t );

--[[ 	t = LoadActor(THEME:GetPathB('', '_frame 3x3'), 'metal', 170, 20) .. {
		Name = 'SmallFrame';
	}; --]]
	t = Def.ActorFrame {
		Name = 'SmallFrame';
--[[ 		Def.Quad {
			InitCommand=cmd(zoomto,170+4,32+4);
			OnCommand=cmd(shadowlength,1);
		}; --]]
		Def.Quad {
			InitCommand=cmd(zoomto,200,40);
			OnCommand=cmd(diffuse,color("0.25,0.5,0.15,1"));
		};
	};
	table.insert( ret, t );

	t = Def.ActorScroller{
		Name = 'Scroller';
		NumItemsToDraw=4;
-- 		InitCommand=cmd(y,-230/2+20;);
		OnCommand=cmd(SetFastCatchup,true;SetMask,200,230;SetSecondsPerItem,0.15);
		TransformFunction=function(self, offset, itemIndex, numItems)
			local focus = scale(math.abs(offset),0,2,1,0);
			self:visible(false);
			self:y(math.floor( offset*40 ));
-- 			self:zoomy( focus );
-- 			self:z(-math.abs(offset));
-- 			self:zoom(focus);
		end;
		children = GetLocalProfiles();
	};
	table.insert( ret, t );

--[[ 	t = Def.BitmapText {
		OnCommand = cmd(y,160);
		Name = 'SelectedProfileText';
		Font = "Common Normal";
		Text = 'No profile';
	}; --]]
	t = LoadFont("Common Normal") .. {
		Name = 'SelectedProfileText';
		InitCommand=cmd(y,160);
		OnCommand=cmd(shadowlength,1);
	};
	table.insert( ret, t );

	return ret;
end;

function UpdateInternal3(self, Player)
	local pn;
	if Player == PLAYER_1 then
		pn = 1;
	else
		pn = 2;
	end;
	local frame = self:GetChild(string.format('P%uFrame', pn));
	local scroller = frame:GetChild('Scroller');
	local seltext = frame:GetChild('SelectedProfileText');
	local smallframe = frame:GetChild('SmallFrame');

	if GAMESTATE:IsHumanPlayer(Player) then
		frame:visible(true);
		if MEMCARDMAN:GetCardState(Player) == 'MemoryCardState_none' then
--using profile if any
			smallframe:visible(true);
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
					smallframe:visible(false);
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
		frame:visible(false);
	end;	
end;

local t = Def.ActorFrame {

	StorageDevicesChangedMessageCommand=function(self, params)
		self:queuecommand('UpdateInternal2');
	end;

	CodeMessageCommand = function(self, params)
		if params.Name == 'Start' then
			if not GAMESTATE:IsHumanPlayer(params.PlayerNumber) then
				SCREENMAN:GetTopScreen():SetProfileIndex(params.PlayerNumber, -1);
			else
				SCREENMAN:GetTopScreen():Finish();
			end;
		end;
		if params.Name == 'Up' then
			if GAMESTATE:IsHumanPlayer(params.PlayerNumber) then
				local ind = SCREENMAN:GetTopScreen():GetProfileIndex(params.PlayerNumber);
				if ind > 1 then
					if SCREENMAN:GetTopScreen():SetProfileIndex(params.PlayerNumber, ind - 1 ) then
						self:queuecommand('UpdateInternal2');
					end;
				end;
			end;
		end;
		if params.Name == 'Down' then
			if GAMESTATE:IsHumanPlayer(params.PlayerNumber) then
				local ind = SCREENMAN:GetTopScreen():GetProfileIndex(params.PlayerNumber);
				if ind > 0 then
					if SCREENMAN:GetTopScreen():SetProfileIndex(params.PlayerNumber, ind + 1 ) then
						self:queuecommand('UpdateInternal2');
					end;
				end;
			end;
		end;
		if params.Name == 'Back' then
			if GAMESTATE:GetNumPlayersEnabled()==0 then
				SCREENMAN:GetTopScreen():Cancel();
			else
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
			InitCommand=cmd(x,SCREEN_CENTER_X-160;y,SCREEN_CENTER_Y;);
			OnCommand=cmd(hide_if,not GAMESTATE:IsHumanPlayer(PLAYER_1););
			children = LoadPlayerStuff(PLAYER_2);
		};
		Def.ActorFrame {
			Name = 'P2Frame';
			InitCommand=cmd(x,SCREEN_CENTER_X+160;y,SCREEN_CENTER_Y;);
			OnCommand=cmd(hide_if,not GAMESTATE:IsHumanPlayer(PLAYER_2););
			children = LoadPlayerStuff(PLAYER_2);
		};
	};
};

return t;
