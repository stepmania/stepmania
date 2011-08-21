local Color1 = color(Var "Color1");

local a = LoadActor(Var "File1") .. {
	OnCommand=cmd(cropto,SCREEN_WIDTH/2,SCREEN_HEIGHT/2;diffuse,Color1;zoomx,self:GetZoomX()*-1;zoomy,self:GetZoomY()*-1;effectclock,"music");
	GainFocusCommand=cmd(play);
	LoseFocusCommand=cmd(pause);
};

local t = Def.ActorFrame {
	a .. { OnCommand=cmd(x,scale(1,0,4,SCREEN_LEFT,SCREEN_RIGHT);y,scale(1,0,4,SCREEN_TOP,SCREEN_BOTTOM)); };
	a .. { OnCommand=cmd(x,scale(3,0,4,SCREEN_LEFT,SCREEN_RIGHT);y,scale(1,0,4,SCREEN_TOP,SCREEN_BOTTOM)); };
	a .. { OnCommand=cmd(x,scale(1,0,4,SCREEN_LEFT,SCREEN_RIGHT);y,scale(3,0,4,SCREEN_TOP,SCREEN_BOTTOM)); };
	a .. { OnCommand=cmd(x,scale(3,0,4,SCREEN_LEFT,SCREEN_RIGHT);y,scale(3,0,4,SCREEN_TOP,SCREEN_BOTTOM)); };
};

return t;
