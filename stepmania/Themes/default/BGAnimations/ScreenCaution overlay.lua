local t = Def.ActorFrame {
	
	LoadFont("_venacti bold 24px") .. {
		InitCommand=cmd(x,SCREEN_CENTER_X;y,SCREEN_CENTER_Y+40;horizalign,center;shadowlength,0;wrapwidthpixels,500;strokecolor,color("#a200ff");settext,THEME:GetString( "ScreenCaution", "CautionText" ););
	};
};

return t;
