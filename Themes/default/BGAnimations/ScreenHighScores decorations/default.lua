local t = LoadFallbackB();

local StepsType = ToEnumShortString( GAMEMAN:GetFirstStepsTypeForGame(GAMESTATE:GetCurrentGame()) );
local stString = THEME:GetString("StepsType",StepsType);

local NumColumns = THEME:GetMetric(Var "LoadingScreen", "NumColumns");

t[#t+1] = Def.ActorFrame {
	InitCommand=cmd(x,SCREEN_CENTER_X;y,SCREEN_CENTER_Y-160);
	Def.Quad {
		InitCommand=cmd(zoomto,SCREEN_WIDTH, 56);
		OnCommand=cmd(diffuse,color("#333333");diffusealpha,0.75;fadebottom,0.35;fadetop,0.35);
	};
};

for i=1,NumColumns do
	local st = THEME:GetMetric(Var "LoadingScreen","ColumnStepsType" .. i);	
	local dc = THEME:GetMetric(Var "LoadingScreen","ColumnDifficulty" .. i);
	local s = GetCustomDifficulty( st, dc );
	
	t[#t+1] = Def.ActorFrame {
		InitCommand=cmd(x,SCREEN_CENTER_X-60 + 80 * (i-1);y,SCREEN_CENTER_Y-160);
		LoadActor("_difficulty frame") .. {
			OnCommand=cmd(diffuse,CustomDifficultyToColor(s);zoomtowidth,80);
		};
		LoadFont("Common Normal") .. {
			InitCommand=cmd(uppercase,true;settext,CustomDifficultyToLocalizedString(s));
			OnCommand=cmd(zoom,0.75;diffuse,CustomDifficultyToColor(s);shadowlength,1);
		};
	};
end

t[#t+1] = LoadFont("Common Bold") .. {
	InitCommand=cmd(settext,stString;x,SCREEN_CENTER_X-220;y,SCREEN_CENTER_Y-160);
	OnCommand=cmd(zoom,0.75;skewx,-0.125;shadowlength,1);
};

t.OnCommand=cmd(draworder,105);

return t;