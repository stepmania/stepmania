local t = Def.ActorFrame {
	Def.Quad{
		OnCommand=cmd(x,SCREEN_CENTER_X;y,SCREEN_CENTER_Y;zoomto,SCREEN_WIDTH,120;diffuse,color("0,0,0,0.7"));
	};
	LoadFont("_terminator two 40px") .. {
		InitCommand=cmd(x,SCREEN_CENTER_X;y,SCREEN_CENTER_Y;shadowlength,0;strokecolor,color("#00006688");settext,"DEMONSTRATION";diffuseshift;effectperiod,1;effectcolor1,color("#A38B00");effectcolor2,color("#F3CB00"););
	};
};

return t;
