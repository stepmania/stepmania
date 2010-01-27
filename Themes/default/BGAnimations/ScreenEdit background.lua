local t = Def.ActorFrame {
   InitCommand=cmd(fov,90);
};
t[#t+1] = Def.ActorFrame {
  InitCommand=cmd(Center);
	Def.Quad {
		InitCommand=cmd(scaletoclipped,SCREEN_WIDTH,SCREEN_HEIGHT);
		OnCommand=cmd(diffuse,color("#ffcb05"));
	};
	LoadActor( THEME:GetPathB("ScreenWithMenuElements","background/_bg top") ) .. {
		InitCommand=cmd(scaletoclipped,SCREEN_WIDTH,SCREEN_HEIGHT);
	};
};

return t;
