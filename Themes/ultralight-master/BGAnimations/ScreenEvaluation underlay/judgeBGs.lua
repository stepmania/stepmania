-- backgrounds AND labels (again because I'm lazy)
local bgWidth = 256; -- was 256
local bgHeight = 22; -- was 24

local labelZoom = 0.75;
local labelOffsetX = 127; -- was 64
local labelAlpha = 0.625; -- was 0.8
local labelSkew = -0.1;

local t = Def.ActorFrame{};

t[#t+1] = Def.Quad{
	Name="P1TopSep";
	InitCommand=cmd(x,met(Var "LoadingScreen","JudgmentP1X");y,SCREEN_CENTER_Y*0.83;zoomto,bgWidth,4;fadeleft,0.35;faderight,0.5;blend,Blend.Add;diffusealpha,0.35);
	BeginCommand=function(self)
		self:visible(IsPlayerValid(PLAYER_1));
	end;
};

t[#t+1] = Def.Quad{
	Name="P2TopSep";
	InitCommand=cmd(x,met(Var "LoadingScreen","JudgmentP2X");y,SCREEN_CENTER_Y*0.83;zoomto,bgWidth,4;fadeleft,0.5;faderight,0.35;blend,Blend.Add;diffusealpha,0.35);
	BeginCommand=function(self)
		self:visible(IsPlayerValid(PLAYER_2));
	end;
};

t[#t+1] = Def.ActorFrame{
	Name="Player1BGs";
	BeginCommand=function(self)
		self:visible(IsPlayerValid(PLAYER_1));
	end;
	Def.Quad{
		Name="W1BG";
		InitCommand=cmd(xy,met(Var "LoadingScreen","JudgmentP1X"),met(Var "LoadingScreen","W1Y");zoomto,bgWidth,bgHeight;diffuse,color("0,0,0,0.625");diffuseleftedge,TapNoteScoreToColor('TapNoteScore_W1'););
	};
	LoadFont("Common normal")..{
		Text=THEME:GetString("TapNoteScore","W1");
		InitCommand=cmd(xy,met(Var "LoadingScreen","JudgmentP1X")-labelOffsetX,met(Var "LoadingScreen","W1Y");halign,0;diffuse,color("0.2,0.2,0.2,"..labelAlpha);zoom,labelZoom;skewx,labelSkew;strokecolor,color("0,0,0,0");shadowlength,0);
	};
	--~~~~~~~--
	Def.Quad{
		Name="W2BG";
		InitCommand=cmd(xy,met(Var "LoadingScreen","JudgmentP1X"),met(Var "LoadingScreen","W2Y");zoomto,bgWidth,bgHeight;diffuse,color("0,0,0,0.625");diffuseleftedge,TapNoteScoreToColor('TapNoteScore_W2'););
	};
	LoadFont("Common normal")..{
		Text=THEME:GetString("TapNoteScore","W2");
		InitCommand=cmd(xy,met(Var "LoadingScreen","JudgmentP1X")-labelOffsetX,met(Var "LoadingScreen","W2Y");halign,0;diffuse,color("0.2,0.2,0.2,"..labelAlpha);zoom,labelZoom;skewx,labelSkew;strokecolor,color("0,0,0,0");shadowlength,0);
	};
	--~~~~~~~--
	Def.Quad{
		Name="W3BG";
		InitCommand=cmd(xy,met(Var "LoadingScreen","JudgmentP1X"),met(Var "LoadingScreen","W3Y");zoomto,bgWidth,bgHeight;diffuse,color("0,0,0,0.625");diffuseleftedge,TapNoteScoreToColor('TapNoteScore_W3'););
	};
	LoadFont("Common normal")..{
		Text=THEME:GetString("TapNoteScore","W3");
		InitCommand=cmd(xy,met(Var "LoadingScreen","JudgmentP1X")-labelOffsetX,met(Var "LoadingScreen","W3Y");halign,0;diffuse,color("0.2,0.2,0.2,"..labelAlpha);zoom,labelZoom;skewx,labelSkew;strokecolor,color("0,0,0,0");shadowlength,0);
	};
	--~~~~~~~--
	Def.Quad{
		Name="W4BG";
		InitCommand=cmd(xy,met(Var "LoadingScreen","JudgmentP1X"),met(Var "LoadingScreen","W4Y");zoomto,bgWidth,bgHeight;diffuse,color("0,0,0,0.625");diffuseleftedge,TapNoteScoreToColor('TapNoteScore_W4'););
	};
	LoadFont("Common normal")..{
		Text=THEME:GetString("TapNoteScore","W4");
		InitCommand=cmd(xy,met(Var "LoadingScreen","JudgmentP1X")-labelOffsetX,met(Var "LoadingScreen","W4Y");halign,0;diffuse,color("0.2,0.2,0.2,"..labelAlpha);zoom,labelZoom;skewx,labelSkew;strokecolor,color("0,0,0,0");shadowlength,0);
	};
	--~~~~~~~--
	Def.Quad{
		Name="W5BG";
		InitCommand=cmd(xy,met(Var "LoadingScreen","JudgmentP1X"),met(Var "LoadingScreen","W5Y");zoomto,bgWidth,bgHeight;diffuse,color("0,0,0,0.625");diffuseleftedge,TapNoteScoreToColor('TapNoteScore_W5'););
	};
	LoadFont("Common normal")..{
		Text=THEME:GetString("TapNoteScore","W5");
		InitCommand=cmd(xy,met(Var "LoadingScreen","JudgmentP1X")-labelOffsetX,met(Var "LoadingScreen","W5Y");halign,0;diffuse,color("0.2,0.2,0.2,"..labelAlpha);zoom,labelZoom;skewx,labelSkew;strokecolor,color("0,0,0,0");shadowlength,0);
	};
	--~~~~~~~--
	Def.Quad{
		Name="MissBG";
		InitCommand=cmd(xy,met(Var "LoadingScreen","JudgmentP1X"),met(Var "LoadingScreen","MissY");zoomto,bgWidth,bgHeight;diffuse,color("0,0,0,0.625");diffuseleftedge,TapNoteScoreToColor('TapNoteScore_Miss'););
	};
	LoadFont("Common normal")..{
		Text=THEME:GetString("TapNoteScore","Miss");
		InitCommand=cmd(xy,met(Var "LoadingScreen","JudgmentP1X")-labelOffsetX,met(Var "LoadingScreen","MissY");halign,0;diffuse,color("0.2,0.2,0.2,"..labelAlpha);zoom,labelZoom;skewx,labelSkew;strokecolor,color("0,0,0,0");shadowlength,0);
	};
};

t[#t+1] = Def.ActorFrame{
	Name="Player2BGs";
	BeginCommand=function(self)
		self:visible(IsPlayerValid(PLAYER_2));
	end;
	Def.Quad{
		Name="W1BG";
		InitCommand=cmd(xy,met(Var "LoadingScreen","JudgmentP2X"),met(Var "LoadingScreen","W1Y");zoomto,bgWidth,bgHeight;diffuse,color("0,0,0,0.625");diffuserightedge,TapNoteScoreToColor('TapNoteScore_W1'););
	};
	LoadFont("Common normal")..{
		Text=THEME:GetString("TapNoteScore","W1");
		InitCommand=cmd(xy,met(Var "LoadingScreen","JudgmentP2X")+labelOffsetX,met(Var "LoadingScreen","W1Y");halign,1;diffuse,color("0.2,0.2,0.2,"..labelAlpha);zoom,labelZoom;skewx,labelSkew;strokecolor,color("0,0,0,0");shadowlength,0);
	};
	--~~~~~~~--
	Def.Quad{
		Name="W2BG";
		InitCommand=cmd(xy,met(Var "LoadingScreen","JudgmentP2X"),met(Var "LoadingScreen","W2Y");zoomto,bgWidth,bgHeight;diffuse,color("0,0,0,0.625");diffuserightedge,TapNoteScoreToColor('TapNoteScore_W2'););
	};
	LoadFont("Common normal")..{
		Text=THEME:GetString("TapNoteScore","W2");
		InitCommand=cmd(xy,met(Var "LoadingScreen","JudgmentP2X")+labelOffsetX,met(Var "LoadingScreen","W2Y");halign,1;diffuse,color("0.2,0.2,0.2,"..labelAlpha);zoom,labelZoom;skewx,labelSkew;strokecolor,color("0,0,0,0");shadowlength,0);
	};
	--~~~~~~~--
	Def.Quad{
		Name="W3BG";
		InitCommand=cmd(xy,met(Var "LoadingScreen","JudgmentP2X"),met(Var "LoadingScreen","W3Y");zoomto,bgWidth,bgHeight;diffuse,color("0,0,0,0.625");diffuserightedge,TapNoteScoreToColor('TapNoteScore_W3'););
	};
	LoadFont("Common normal")..{
		Text=THEME:GetString("TapNoteScore","W3");
		InitCommand=cmd(xy,met(Var "LoadingScreen","JudgmentP2X")+labelOffsetX,met(Var "LoadingScreen","W3Y");halign,1;diffuse,color("0.2,0.2,0.2,"..labelAlpha);zoom,labelZoom;skewx,labelSkew;strokecolor,color("0,0,0,0");shadowlength,0);
	};
	--~~~~~~~--
	Def.Quad{
		Name="W4BG";
		InitCommand=cmd(xy,met(Var "LoadingScreen","JudgmentP2X"),met(Var "LoadingScreen","W4Y");zoomto,bgWidth,bgHeight;diffuse,color("0,0,0,0.625");diffuserightedge,TapNoteScoreToColor('TapNoteScore_W4'););
	};
	LoadFont("Common normal")..{
		Text=THEME:GetString("TapNoteScore","W4");
		InitCommand=cmd(xy,met(Var "LoadingScreen","JudgmentP2X")+labelOffsetX,met(Var "LoadingScreen","W4Y");halign,1;diffuse,color("0.2,0.2,0.2,"..labelAlpha);zoom,labelZoom;skewx,labelSkew;strokecolor,color("0,0,0,0");shadowlength,0);
	};
	--~~~~~~~--
	Def.Quad{
		Name="W5BG";
		InitCommand=cmd(xy,met(Var "LoadingScreen","JudgmentP2X"),met(Var "LoadingScreen","W5Y");zoomto,bgWidth,bgHeight;diffuse,color("0,0,0,0.625");diffuserightedge,TapNoteScoreToColor('TapNoteScore_W5'););
	};
	LoadFont("Common normal")..{
		Text=THEME:GetString("TapNoteScore","W5");
		InitCommand=cmd(xy,met(Var "LoadingScreen","JudgmentP2X")+labelOffsetX,met(Var "LoadingScreen","W5Y");halign,1;diffuse,color("0.2,0.2,0.2,"..labelAlpha);zoom,labelZoom;skewx,labelSkew;strokecolor,color("0,0,0,0");shadowlength,0);
	};
	--~~~~~~~--
	Def.Quad{
		Name="MissBG";
		InitCommand=cmd(xy,met(Var "LoadingScreen","JudgmentP2X"),met(Var "LoadingScreen","MissY");zoomto,bgWidth,bgHeight;diffuse,color("0,0,0,0.625");diffuserightedge,TapNoteScoreToColor('TapNoteScore_Miss'););
	};
	LoadFont("Common normal")..{
		Text=THEME:GetString("TapNoteScore","Miss");
		InitCommand=cmd(xy,met(Var "LoadingScreen","JudgmentP2X")+labelOffsetX,met(Var "LoadingScreen","MissY");halign,1;diffuse,color("0.2,0.2,0.2,"..labelAlpha);zoom,labelZoom;skewx,labelSkew;strokecolor,color("0,0,0,0");shadowlength,0);
	};
};

t[#t+1] = Def.Quad{
	Name="P1BottomSep";
	InitCommand=cmd(x,met(Var "LoadingScreen","JudgmentP1X");y,SCREEN_CENTER_Y*1.475;zoomto,bgWidth,4;fadeleft,0.35;faderight,0.5;blend,Blend.Add;diffusealpha,0.35);
	BeginCommand=function(self)
		self:visible(IsPlayerValid(PLAYER_1));
	end;
};

t[#t+1] = Def.Quad{
	Name="P2BottomSep";
	InitCommand=cmd(x,met(Var "LoadingScreen","JudgmentP2X");y,SCREEN_CENTER_Y*1.475;zoomto,bgWidth,4;fadeleft,0.5;faderight,0.35;blend,Blend.Add;diffusealpha,0.35);
	BeginCommand=function(self)
		self:visible(IsPlayerValid(PLAYER_2));
	end;
};

return t;