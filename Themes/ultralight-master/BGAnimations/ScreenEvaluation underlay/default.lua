local t = Def.ActorFrame{};

if ScreenMetric("Summary") then
	-- summary just uses the normal background
	t[#t+1] = Def.Quad{
		InitCommand=cmd(FullScreen;diffuse,HSV(192,1,0.05);diffusebottomedge,HSV(192,0.75,0.125))
	}
elseif GAMESTATE:IsCourseMode() then
	-- course mode
	t[#t+1] = Def.Quad{
		InitCommand=cmd(FullScreen;diffuse,HSVA(0,0.5,0.75,PREFSMAN:GetPreference("BGBrightness"))),
		OnCommand=cmd(accelerate,1;diffuse,HSVA(0,0.5,0.75,0.5))
	}
else
	-- song background
	t[#t+1] = LoadSongBackground()..{
		InitCommand=cmd(diffusealpha,PREFSMAN:GetPreference("BGBrightness")),
		OnCommand=cmd(accelerate,1;diffusealpha,0.5)
	}
end;

local gradeFrameWidth = 156
local gradeFrameHeight = 28
local gradeFrameBorderSize = 2
-- score/grade frames
t[#t+1] = Def.ActorFrame{
	Name="P1ScoreFrame",
	InitCommand=cmd(x,SCREEN_CENTER_X*0.3;y,SCREEN_TOP+120),
	BeginCommand=cmd(hide_if,not IsPlayerValid(PLAYER_1)),
	Def.Quad{
		Name="Outer",
		InitCommand=cmd(zoomto,gradeFrameWidth+(gradeFrameBorderSize),gradeFrameHeight+(gradeFrameBorderSize);diffuse,color("0,0,0,0.5"))
	},
	Def.Quad{
		Name="Inner",
		InitCommand=cmd(x,-0.125;y,0.25;zoomto,gradeFrameWidth,gradeFrameHeight;diffuse,Brightness(PlayerColor(PLAYER_1),0.75))
	},
	Def.Quad{
		Name="GradePart",
		InitCommand=cmd(x,gradeFrameWidth*0.5;y,0.25;halign,1;zoomto,gradeFrameWidth*0.325,gradeFrameHeight;diffuse,HSVA(0,0,0.9,0.8);),
		BeginCommand=function(self)
			self:visible(THEME:GetMetric(Var "LoadingScreen","ShowGradeArea"))
		end
	},
	Def.Quad{
		Name="Under",
		InitCommand=cmd(y,20;zoomto,gradeFrameWidth+(gradeFrameBorderSize),4;fadeleft,0.35;faderight,0.35;diffusealpha,0.45;blend,Blend.Add)
	},
	Def.Quad{
		Name="Under2",
		InitCommand=cmd(y,17;zoomto,gradeFrameWidth+(gradeFrameBorderSize),2;fadeleft,0.25;faderight,0.25;diffusealpha,0.625;blend,Blend.Add;skewx,-0.05)
	},
}

t[#t+1] = Def.ActorFrame{
	Name="P2ScoreFrame",
	InitCommand=cmd(x,SCREEN_CENTER_X*1.7;y,SCREEN_TOP+120),
	BeginCommand=cmd(hide_if,not IsPlayerValid(PLAYER_2)),
	Def.Quad{
		Name="Outer",
		InitCommand=cmd(zoomto,gradeFrameWidth+(gradeFrameBorderSize),gradeFrameHeight+(gradeFrameBorderSize);diffuse,color("0,0,0,0.5"))
	},
	Def.Quad{
		Name="Inner",
		InitCommand=cmd(x,-0.125;y,0.25;zoomto,gradeFrameWidth,gradeFrameHeight;diffuse,Brightness(PlayerColor(PLAYER_2),0.75))
	},
	Def.Quad{
		Name="GradePart",
		InitCommand=cmd(x,gradeFrameWidth*0.5;y,0.25;halign,1;zoomto,gradeFrameWidth*0.325,gradeFrameHeight;diffuse,HSVA(0,0,0.9,0.8);),
		BeginCommand=function(self)
			self:visible(THEME:GetMetric(Var "LoadingScreen","ShowGradeArea"))
		end
	},
	Def.Quad{
		Name="Under",
		InitCommand=cmd(y,20;zoomto,gradeFrameWidth+(gradeFrameBorderSize),4;fadeleft,0.35;faderight,0.35;diffusealpha,0.45;blend,Blend.Add)
	},
	Def.Quad{
		Name="Under2",
		InitCommand=cmd(y,17;zoomto,gradeFrameWidth+(gradeFrameBorderSize),2;fadeleft,0.25;faderight,0.25;diffusealpha,0.625;blend,Blend.Add;skewx,-0.05)
	}
}

-- decoration lines
local middleX = WideScale(90,128)
local bannerSideX = WideScale(136,174)
local endSideX = WideScale(44,82)

t[#t+1] = Def.ActorFrame{
	Name="P1LineFrame",
	InitCommand=cmd(hide_if,not IsPlayerValid(PLAYER_1)),
	Def.Quad{
		Name="LeftPart1",
		InitCommand=cmd(x,SCREEN_CENTER_X-bannerSideX;y,SCREEN_TOP+70;zoomto,4,146;cropbottom,1;diffusetopedge,PlayerColor(PLAYER_1)),
		OnCommand=cmd(decelerate,0.5;cropbottom,0)
	},
	Def.Quad{
		Name="LeftPart2",
		InitCommand=cmd(x,SCREEN_CENTER_X-middleX;y,SCREEN_TOP+144;zoomto,96,4;cropright,1),
		OnCommand=cmd(sleep,0.5;accelerate,0.5;cropright,0)
	},
	Def.Quad{
		Name="LeftPart3",
		InitCommand=cmd(x,SCREEN_CENTER_X-endSideX;y,SCREEN_CENTER_Y*1.3425;zoomto,4,SCREEN_HEIGHT*0.75;diffusebottomedge,PlayerColor(PLAYER_1);cropbottom,1),
		OnCommand=cmd(sleep,1;decelerate,0.5;cropbottom,0)
	},
}

t[#t+1] = Def.ActorFrame{
	Name="P2LineFrame",
	InitCommand=cmd(hide_if,not IsPlayerValid(PLAYER_2)),
	Def.Quad{
		Name="RightPart1",
		InitCommand=cmd(x,SCREEN_CENTER_X+bannerSideX;y,SCREEN_TOP+70;zoomto,4,146;cropbottom,1;diffusetopedge,PlayerColor(PLAYER_2)),
		OnCommand=cmd(decelerate,0.5;cropbottom,0)
	},
	Def.Quad{
		Name="RightPart2",
		InitCommand=cmd(x,SCREEN_CENTER_X+middleX;y,SCREEN_TOP+144;zoomto,96,4;cropleft,1),
		OnCommand=cmd(sleep,0.5;accelerate,0.5;cropleft,0)
	},
	Def.Quad{
		Name="RightPart3",
		InitCommand=cmd(x,SCREEN_CENTER_X+endSideX;y,SCREEN_CENTER_Y*1.3425;zoomto,4,SCREEN_HEIGHT*0.75;diffusebottomedge,PlayerColor(PLAYER_2);cropbottom,1),
		OnCommand=cmd(sleep,1;decelerate,0.5;cropbottom,0)
	},
}

t[#t+1] = LoadActor("judgeBGs")

return t
