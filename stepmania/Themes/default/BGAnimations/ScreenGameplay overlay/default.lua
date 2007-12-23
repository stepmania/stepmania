local font = LoadFont( "_zeroesthree" );
local frame = LoadActor( "_score frame" );
local text = GAMESTATE:GetPlayMode() == 'PlayMode_Oni' and "time" or "score";

local function DifficultyLabelText(pn)
	local yNormal = SCREEN_CENTER_Y+175;
	local yReverse = SCREEN_CENTER_Y-189;
	local xNormal = 286;
	local xReverse = 310;

	local diffText = font .. {
		InitCommand=function(self)
			-- check to see if player is using reverse and set position accordingly.
			if GAMESTATE:PlayerIsUsingModifier(pn,"reverse") then
				if pn == PLAYER_1 then
					-- player 1 reverse position
					self:x(SCREEN_CENTER_X-xReverse);
					self:y(yReverse);
				else
					-- player 2 reverse position
					self:x(SCREEN_CENTER_X+xReverse);
					self:y(yReverse);
				end;
			else
				if pn == PLAYER_1 then
					-- player 1 normal position
					self:x(SCREEN_CENTER_X-xNormal);
					self:y(yNormal);
				else
					-- player 2 normal position
					self:x(SCREEN_CENTER_X+xNormal);
					self:y(yNormal);
				end;
			end;
			
			-- set align based on player
			if pn == PLAYER_1 then self:horizalign('HorizAlign_Left');
			else self:horizalign('HorizAlign_Right');
			end;
			
			-- other commands
			self:zoom(0.75);
			self:shadowlength(0);
		end;
		OnCommand=cmd(playcommand,'Update');
		CurrentStepsP1ChangedMessageCommand=cmd(playcommand,'Update');
		CurrentStepsP2ChangedMessageCommand=cmd(playcommand,'Update');
		UpdateCommand=function(self)
			self:settext(DifficultyToLocalizedString(GAMESTATE:GetCurrentSteps(pn):GetDifficulty()));
		end;
		Condition=GAMESTATE:IsSideJoined(pn) == true or GAMESTATE:GetPlayMode() == 'PlayMode_Rave';
	};
	
	return diffText;
end;

return Def.ActorFrame {

	-- text on difficulty labels
	DifficultyLabelText(PLAYER_1);
	DifficultyLabelText(PLAYER_2);

	-- text on bottom of score frames
	Def.ActorFrame {
		frame;
		font .. {
			Text = "player 1 " .. text;
			InitCommand = cmd(horizalign,left;addy,16;addx,-80;zoom,0.75;shadowlength,0);
		};
		InitCommand = cmd(x,SCREEN_LEFT-100;y,SCREEN_CENTER_Y+202);
		OnCommand = cmd(linear,1;x,SCREEN_CENTER_X-208);
		OffCommand = cmd(linear,1;x,SCREEN_LEFT-100);
		Condition=GAMESTATE:IsSideJoined(PLAYER_1) == true or GAMESTATE:GetPlayMode() == 'PlayMode_Rave';
	};
	Def.ActorFrame {
		frame;
		font .. {
			Text = "player 2 " .. text;
			InitCommand = cmd(horizalign,right;addy,16;addx,80;zoom,0.75;shadowlength,0);
		};
		InitCommand = cmd(x,SCREEN_RIGHT+100;y,SCREEN_CENTER_Y+202);
		OnCommand = cmd(linear,1;x,SCREEN_CENTER_X+210);
		OffCommand = cmd(linear,1;x,SCREEN_RIGHT+100);
		Condition=GAMESTATE:IsSideJoined(PLAYER_2) == true or GAMESTATE:GetPlayMode() == 'PlayMode_Rave';
	}
};
