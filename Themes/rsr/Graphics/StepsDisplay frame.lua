return Def.ActorFrame {
	Def.Quad {
		InitCommand=cmd(x,-(32)/2;zoomto,128+32,32);
	};
	Def.Quad {
		InitCommand=cmd(x,(128+32)/2;zoomto,32,32;diffuse,color("#7C7C7C"));
	};
};
