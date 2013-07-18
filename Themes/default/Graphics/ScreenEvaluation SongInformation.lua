return Def.ActorFrame {
 	LoadFont("Common Normal") .. {
		Name="TextTitle";
		InitCommand=cmd(y,-16.5;zoom,0.875;maxwidth,256/0.875;);
		OnCommand=cmd(shadowlength,1);
-- 		TickCommand=cmd(finishtweening;diffusealpha,0;addx,-10;zoomx,1.25;zoomy,0;decelerate,0.25;diffusealpha,1;addx,10;zoom,1;sleep,0;glow,Color("White");decelerate,0.275;glow,Color("Invisible"));
	};
 	LoadFont("Common Normal") .. {
		Name="TextSubtitle";
		InitCommand=cmd(zoom,0.5;maxwidth,256/0.5);
		OnCommand=cmd(shadowlength,1);
-- 		TickCommand=cmd(finishtweening;diffusealpha,0;addy,-10;addx,10;decelerate,0.25;diffusealpha,1;addy,10;addx,-10);
	};
	LoadFont("Common Normal") .. {
		Name="TextArtist";
		InitCommand=cmd(y,18;zoom,0.75;maxwidth,256/0.75);
		OnCommand=cmd(shadowlength,1;skewx,-0.2);
-- 		TickCommand=cmd(finishtweening;diffusealpha,0;addy,10;addx,10;decelerate,0.25;diffusealpha,1;addy,-10;addx,-10);
	};
};