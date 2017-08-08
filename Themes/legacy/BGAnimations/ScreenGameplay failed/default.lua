local longFail = ThemePrefs.Get("LongFail");

local t = Def.ActorFrame{};

if longFail then
	t[#t+1] = Def.Quad{
		InitCommand=cmd(FullScreen;diffuse,color("1,0,0,0");blend,Blend.Multiply);
		OnCommand=cmd(decelerate,1.25;diffuse,color("0.75,0,0,0.75");linear,7;diffuse,color("0,0,0,1");sleep,1.25;linear,1;diffuse,color("1,0,0,1");decelerate,2;diffuse,color("0,0,0,1"));
	};
	t[#t+1] = Def.Quad{
		InitCommand=cmd(FullScreen;diffuse,color("1,1,1,1");diffusealpha,0);
		OnCommand=cmd(finishtweening;diffusealpha,1;decelerate,1.25;diffuse,color("1,0,0,0"));
	};
	t[#t+1] = LoadActor(THEME:GetPathS( "ScreenGameplayAlternate", "failed" ) ) .. {
		StartTransitioningCommand=cmd(play);
	};
else
	t[#t+1] = Def.Quad{
		InitCommand=cmd(FullScreen;diffuse,color("1,0,0,0");blend,Blend.Multiply);
		OnCommand=cmd(smooth,1;diffuse,color("0.75,0,0,0.75");decelerate,2;diffuse,color("0,0,0,1"));
	};
	t[#t+1] = Def.Quad{
		InitCommand=cmd(FullScreen;diffuse,color("1,1,1,1");diffusealpha,0);
		OnCommand=cmd(finishtweening;diffusealpha,1;decelerate,1.25;diffuse,color("1,0,0,0"));
	};
	t[#t+1] = LoadActor(THEME:GetPathS( Var "LoadingScreen", "failed" ) ) .. {
		StartTransitioningCommand=cmd(play);
	};
end;

return t;