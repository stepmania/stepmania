local t = LoadFallbackB();

local StepsType = ToEnumShortString( GAMEMAN:GetFirstStepsTypeForGame(GAMESTATE:GetCurrentGame()) );
local stString = THEME:GetString("StepsType",StepsType);

local NumColumns = THEME:GetMetric(Var "LoadingScreen", "NumColumns");

t[#t+1] = Def.ActorFrame {
	InitCommand=cmd(x,SCREEN_CENTER_X;y,SCREEN_CENTER_Y-160);
	Def.Quad {
		InitCommand=cmd(zoomto,SCREEN_WIDTH, 32);
		OnCommand=cmd(y,-16;diffuse,Color.Black;fadebottom,0.8);
	};
	Def.Quad {
		InitCommand=cmd(zoomto,SCREEN_WIDTH, 56);
		OnCommand=cmd(diffuse,color("#333333");diffusealpha,0.75;fadebottom,0.35);
	};
};

for i=1,NumColumns do
	local st = THEME:GetMetric(Var "LoadingScreen","ColumnStepsType" .. i);	
	local dc = THEME:GetMetric(Var "LoadingScreen","ColumnDifficulty" .. i);
	local s = GetCustomDifficulty( st, dc );
	
	t[#t+1] = Def.ActorFrame {
		InitCommand=cmd(x,SCREEN_CENTER_X-60 + 80 * (i-1);y,SCREEN_CENTER_Y-168);
		LoadActor(THEME:GetPathB("_frame","3x1"),"rounded fill", 18) .. {
			OnCommand=cmd(diffuse,CustomDifficultyToDarkColor(s);diffusealpha,0.5);
		};
		LoadActor(THEME:GetPathB("_frame","3x1"),"rounded gloss", 18) .. {
			OnCommand=cmd(diffuse,CustomDifficultyToColor(s);diffusealpha,0.125);
		};
		LoadFont("Common Normal") .. {
			InitCommand=cmd(uppercase,true;settext,CustomDifficultyToLocalizedString(s));
			OnCommand=cmd(zoom,0.675;maxwidth,80/0.675;diffuse,CustomDifficultyToColor(s);shadowlength,1);
		};
	};
end

t[#t+1] = LoadFont("Common Bold") .. {
	InitCommand=cmd(settext,stString;x,SCREEN_CENTER_X-220;y,SCREEN_CENTER_Y-168);
	OnCommand=cmd(skewx,-0.125;diffusebottomedge,color("0.75,0.75,0.75");shadowlength,2);
};

t.OnCommand=cmd(draworder,105);

return t;
