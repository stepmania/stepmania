local num_players = GAMESTATE:GetHumanPlayers();
local function PositionItem(i,max)
	local x_spacing = 128; 
	return x_spacing * (i-(max-1)/2);
end


local t = Def.ActorFrame {
	FOV=90;
	--
	Def.Quad {
		InitCommand=cmd(zoomto,SCREEN_CENTER_X+80,SCREEN_HEIGHT);
		OnCommand=cmd(diffuse,Color.Black;diffusealpha,0.75;fadeleft,32/SCREEN_CENTER_X;faderight,32/SCREEN_CENTER_X);
	};
};
--
for i=1,#num_players do
	local f = Def.ActorFrame {
		InitCommand=cmd(x,-128+PositionItem(i,#num_players));
		UnchosenCommand=cmd(finishtweening;bounceend,0.25;zoom,1);
		ChosenCommand=cmd(stoptweening;bouncebegin,0.3;zoom,0);
		--
		StepsChosenMessageCommand=function( self, param ) 
			if param.Player ~= num_players[i] then return end;
			self:playcommand("Chosen");
		end;
		StepsUnchosenMessageCommand=function( self, param ) 
			if param.Player ~= num_players[i] then return end;
			self:playcommand("Unchosen");
		end;
		Def.Quad {
			InitCommand=cmd(y,-35);
			OnCommand=cmd(diffuse,PlayerColor(num_players[i]);shadowlength,1;linear,0.25;zoomtowidth,80;fadeleft,0.5;faderight,0.5);
		};
		LoadFont("Common Bold") .. {
			Text=ToEnumShortString(num_players[i]);
			InitCommand=cmd(y,-48);
			OnCommand=cmd(shadowlength,1;diffuse,PlayerColor(num_players[i]));
		};
		LoadFont("Common Bold") .. {
			Text="PRESS";
			InitCommand=cmd(y,-20);
			OnCommand=cmd(shadowlength,1;pulse;effectmagnitude,1,1.125,1;effectperiod,0.5);
		};
		LoadFont("Common Normal") .. {
			Text="TO START";
			InitCommand=cmd(y,58);
			OnCommand=cmd(shadowlength,1;zoom,0.75);
		};
	};
	if GAMESTATE:GetCurrentGame():GetName() == "pump" then
		local ns = num_players[i] == PLAYER_1 and RoutineSkinP1() or RoutineSkinP2()
		f[#f+1] = LoadActor( NOTESKIN:GetPathForNoteSkin("Center","Tap",ns) ) .. {
			InitCommand=cmd(y,20);
		}
	end
	t[#t+1] = f;
end
-- Lock input for half a second so that players don't accidentally start a song
t[#t+1] = Def.Actor { 
	StartSelectingStepsMessageCommand=function() SCREENMAN:GetTopScreen():lockinput(0.5); end;
};

--
t.InitCommand=cmd(Center;x,SCREEN_CENTER_X*1.5;diffusealpha,0);
t.StartSelectingStepsMessageCommand=cmd(linear,0.2;diffusealpha,1);
t.SongUnchosenMessageCommand=cmd(linear,0.2;diffusealpha,0);


return t;
