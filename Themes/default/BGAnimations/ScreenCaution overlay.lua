local t = Def.ActorFrame {
	Def.Quad {
		InitCommand=cmd(x,SCREEN_CENTER_X;y,SCREEN_TOP;vertalign,bottom;diffuse,color("#000000");zoomto,SCREEN_WIDTH,SCREEN_HEIGHT;);
		BeginCommand=cmd(zwrite,1;z,1;blend,"BlendMode_NoEffect");
		OffCommand=cmd(accelerate,.4;y,SCREEN_BOTTOM);
	};
	LoadFont("_venacti bold 24px") .. {
		InitCommand=cmd(x,SCREEN_CENTER_X;y,SCREEN_CENTER_Y+40;horizalign,center;shadowlength,0;wrapwidthpixels,500;strokecolor,color("#a200ff");settext,THEME:GetString( "ScreenCaution", "CautionText" ););
		BeginCommand=cmd(ztest,1);
		--OffCommand=cmd(croptop,0;linear,0.3;croptop,1);  not working, so i used a mask lol
	};
};

return t;
