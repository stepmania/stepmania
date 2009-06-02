return Def.ActorFrame {
	Def.Quad {
		Name="ItemBase";
		InitCommand=cmd(zoomto,270,32;diffuse,color("0,0,0,0.5"););
	};
	Def.Quad {
		Name="ItemBottomBorder";
		InitCommand=cmd(y,16;zoomto,270,2;diffuse,color("1,1,1,0.5"););
	};
};