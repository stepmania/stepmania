local t = Def.ActorFrame {
	
	LoadFont("_venacti bold 50") .. {
		Text=THEME:GetString( "ScreenCaution", "CautionText" );
		InitCommand=cmd(x,SCREEN_CENTER_X;y,SCREEN_CENTER_Y;horizalign,center;shadowlength,0;wrapwidthpixels,640;strokecolor,color("#a200ff"););
	};
};

return t;
