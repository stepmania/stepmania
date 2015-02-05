local Color1 = color(Var "Color1");
local Color2 = color(Var "Color2");

-- Alternating files being played back at once
local t = Def.ActorFrame {
	LoadActor(Var "File1") .. { OnCommand=cmd(x,scale(1,0,4,SCREEN_LEFT,SCREEN_RIGHT);y,scale(1,0,4,SCREEN_TOP,SCREEN_BOTTOM);cropto,SCREEN_WIDTH/2,SCREEN_HEIGHT/2;diffuse,Color1) };
	LoadActor(Var "File2") .. { OnCommand=cmd(x,scale(3,0,4,SCREEN_LEFT,SCREEN_RIGHT);y,scale(1,0,4,SCREEN_TOP,SCREEN_BOTTOM);cropto,SCREEN_WIDTH/2,SCREEN_HEIGHT/2;diffuse,Color2) };
	LoadActor(Var "File2") .. {
		OnCommand=function(self)
			self:x(scale(1,0,4,SCREEN_LEFT,SCREEN_RIGHT)):y(scale(3,0,4,SCREEN_TOP,SCREEN_BOTTOM)):cropto(SCREEN_WIDTH/2,SCREEN_HEIGHT/2):diffuse(Color1)
			if self.SetDecodeMovie then self:SetDecodeMovie(false) end
														end };
	LoadActor(Var "File1") .. {
		OnCommand=function(self)
			self:x(scale(3,0,4,SCREEN_LEFT,SCREEN_RIGHT)):y(scale(3,0,4,SCREEN_TOP,SCREEN_BOTTOM)):cropto(SCREEN_WIDTH/2,SCREEN_HEIGHT/2):diffuse(Color2)
			if self.SetDecodeMovie then self:SetDecodeMovie(false) end
														end };
};

return t
