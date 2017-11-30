local t = LoadFallbackB();

local StepsType = ToEnumShortString( GAMEMAN:GetFirstStepsTypeForGame(GAMESTATE:GetCurrentGame()) );
local stString = THEMEMAN:GetString("StepsType",StepsType);

local NumColumns = THEMEMAN:GetMetric(Var "LoadingScreen", "NumColumns");

t[#t+1] = Def.ActorFrame {
	InitCommand=cmd(x,SCREEN_CENTER_X;y,SCREEN_CENTER_Y-220);
	Def.Quad {
		InitCommand=cmd(zoomto,SCREEN_WIDTH, 56);
		OnCommand=function(self)
			self:diffuse(ScreenColor(SCREENMAN:GetTopScreen():GetName())):diffusebottomedge(ColorDarkTone(ScreenColor(SCREENMAN:GetTopScreen():GetName()))):diffusealpha(0.9)
		end
	};
};

for i=1,NumColumns do
	local st = THEMEMAN:GetMetric(Var "LoadingScreen","ColumnStepsType" .. i);	
	local dc = THEMEMAN:GetMetric(Var "LoadingScreen","ColumnDifficulty" .. i);
	local s = GetCustomDifficulty( st, dc );
	
	t[#t+1] = Def.ActorFrame {
		InitCommand=cmd(x,SCREEN_CENTER_X+32 + 84 * (i-1);y,SCREEN_CENTER_Y-220);
		LoadActor(THEMEMAN:GetPathB("_frame","3x1"),"rounded light", 18) .. {
			OnCommand=cmd(diffuse,CustomDifficultyToLightColor(s);diffusealpha,0.9);
		};
		LoadFont("StepsDisplayListRow description") .. {
			InitCommand=cmd(uppercase,true;settext,CustomDifficultyToLocalizedString(s));
			OnCommand=cmd(zoom,0.675;maxwidth,80/0.675;diffuse,CustomDifficultyToDarkColor(s););
		};
	};
end

t[#t+1] = LoadFont("Common Normal") .. {
	InitCommand=cmd(settext,stString;x,SCREEN_CENTER_X-220;y,SCREEN_CENTER_Y-220;);
	OnCommand=cmd(diffusebottomedge,color("0.75,0.75,0.75");shadowlength,2);
};

t.OnCommand=cmd(draworder,105);

return t;
