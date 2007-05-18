return Def.ActorFrame {
	LoadActor( THEME:GetPathS("", "_cancel") ) .. {
		StartTransitioningCommand=cmd(play);
	};
	Def.Quad {
		OnCommand=cmd(finishtweening;diffuse,color("#000000");stretchto,SCREEN_LEFT,SCREEN_TOP,SCREEN_RIGHT,SCREEN_BOTTOM;cropleft,1;fadeleft,.5;linear,0.5;cropleft,-0.5);
	};
};
