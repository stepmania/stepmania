local t = Def.ActorFrame {
	LoadFont("blaster") .. {
		Text=THEME:GetString( "ScreenCaution", "Caution" );
		InitCommand=cmd(x,SCREEN_CENTER_X;y,SCREEN_TOP+60;diffuse,1,0,0,1;zoom,1.5;diffusebottomedge,0.25,0,0,1;shadowlength,0);
	};
	
	LoadFont("_zeroesthree") .. {
		Text=THEME:GetString( "ScreenCaution", "CautionText" );
		InitCommand=cmd(x,SCREEN_CENTER_X;y,SCREEN_CENTER_Y;horizalign,center;shadowlength,0;wrapwidthpixels,SCREEN_WIDTH-80);
	};
};

return t;
