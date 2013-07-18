local Color1 = color(Var "Color1");
local Color2 = color(Var "Color2");

local a1 = LoadActor(Var "File1") .. {
	OnCommand=cmd(cropto,SCREEN_WIDTH/2;zoomtoheight,SCREEN_HEIGHT/2;diffuse,Color1;effectclock,"music");
	GainFocusCommand=cmd(play);
	LoseFocusCommand=cmd(pause);
};
local a2 = LoadActor(Var "File2") .. {
	OnCommand=cmd(cropto,SCREEN_WIDTH/2;zoomtoheight,SCREEN_HEIGHT/2;diffuse,Color2;effectclock,"music");
	GainFocusCommand=cmd(play);
	LoseFocusCommand=cmd(pause);
};

local t = Def.ActorFrame {
	a1 .. { OnCommand=cmd(x,scale(1,0,4,SCREEN_LEFT,SCREEN_RIGHT);y,scale(1,0,4,SCREEN_TOP,SCREEN_BOTTOM)); };
	a2 .. { OnCommand=cmd(x,scale(3,0,4,SCREEN_LEFT,SCREEN_RIGHT);y,scale(1,0,4,SCREEN_TOP,SCREEN_BOTTOM)); };
	a2 .. { OnCommand=cmd(x,scale(1,0,4,SCREEN_LEFT,SCREEN_RIGHT);y,scale(3,0,4,SCREEN_TOP,SCREEN_BOTTOM)); };
	a1 .. { OnCommand=cmd(x,scale(3,0,4,SCREEN_LEFT,SCREEN_RIGHT);y,scale(3,0,4,SCREEN_TOP,SCREEN_BOTTOM)); };
};

return t;

