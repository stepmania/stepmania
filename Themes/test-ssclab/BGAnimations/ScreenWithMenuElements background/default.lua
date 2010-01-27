t = Def.ActorFrame { FOV = 90; };
-- BG Frame
t[#t+1] = Def.ActorFrame {
	InitCommand=cmd(z,-48);
	LoadActor("noise");
	-- Centerize Items Here
	Def.ActorFrame {
		InitCommand=cmd(x,SCREEN_CENTER_X;y,SCREEN_CENTER_Y;);
		Def.Quad {
			InitCommand=cmd(zoomto,SCREEN_WIDTH+256,SCREEN_WIDTH+256;blend,'BlendMode_WeightedMultiply');
			OnCommand=cmd(diffuse,Color.ThemeElement.Background.Secondary;);
		};
	};
	-- Radio Lines
	Def.ActorFrame {
		InitCommand=cmd(x,SCREEN_CENTER_X;y,SCREEN_CENTER_Y;);
		LoadActor("stripe") .. {
			InitCommand=cmd(zoomto,SCREEN_WIDTH+512,Viewport.Height.Center);
			OnCommand=cmd(diffuse,Color.ThemeElement.Background.Primary;diffusealpha,0.5;fadetop,0.125;fadebottom,0.125;customtexturerect,(SCREEN_WIDTH+512)/64,Viewport.Height.Center/64,0,0;texcoordvelocity,1.25,0;skewx,-0.125);
		};
	};
};
-- Movie
t[#t+1] = Def.ActorFrame {
	InitCommand=cmd(x,SCREEN_CENTER_X;y,SCREEN_CENTER_Y;);
	LoadActor("824_JumpBack") .. {
		InitCommand=cmd(scaletoclipped,SCREEN_WIDTH,Viewport.Height.Center;
			play;rate,1.75;fadetop,0.125;fadebottom,0.125;diffuse,Color.ThemeElement.Background.Primary;diffusealpha,0.5);
	};
};
-- Front Windows
t[#t+1] = Def.ActorFrame {
	InitCommand=cmd(z,0);
	-- Top Window.
	Def.ActorFrame {
		InitCommand=cmd(x,SCREEN_CENTER_X;y,SCREEN_TOP);
		OnCommand=cmd(decelerate,0.5;y,Viewport.Height.Bottom);
		Def.Quad {
			InitCommand=cmd(zoomto,SCREEN_WIDTH,Viewport.Height.Bottom;vertalign,bottom);
			OnCommand=cmd(diffuse,Color.ThemeElement.Background.Primary);
		};
		Def.Quad {
			InitCommand=cmd(zoomto,SCREEN_WIDTH,16;y,7;vertalign,bottom);
			OnCommand=cmd(	diffuse,Color.ThemeElement.Background.Highlight;fadetop,0.5;fadebottom,0.5;
							diffuseshift;effectcolor1,Color.ThemeElement.Background.Highlight;effectcolor2,Color.ThemeElement.Background.Accent;
							diffusealpha,0.5);
		};
		Def.Quad {
			InitCommand=cmd(zoomto,SCREEN_WIDTH,2;vertalign,bottom);
			OnCommand=cmd(diffuse,Color.ThemeElement.Background.Highlight);
		};
	};
	-- Bottom Window.
	Def.ActorFrame {
		InitCommand=cmd(x,SCREEN_CENTER_X;y,SCREEN_BOTTOM);
		OnCommand=cmd(decelerate,0.5;y,SCREEN_BOTTOM-Viewport.Height.Bottom);
		Def.Quad {
			InitCommand=cmd(zoomto,SCREEN_WIDTH,Viewport.Height.Bottom;vertalign,top);
			OnCommand=cmd(diffuse,Color.ThemeElement.Background.Primary);
		};
		Def.Quad {
			InitCommand=cmd(zoomto,SCREEN_WIDTH,16;y,-7;vertalign,top);
			OnCommand=cmd(	diffuse,Color.ThemeElement.Background.Highlight;fadetop,0.5;fadebottom,0.5;
							diffuseshift;effectcolor1,Color.ThemeElement.Background.Highlight;effectcolor2,Color.ThemeElement.Background.Accent;
							diffusealpha,0.5);
		};
		Def.Quad {
			InitCommand=cmd(zoomto,SCREEN_WIDTH,2;vertalign,top);
			OnCommand=cmd(diffuse,Color.ThemeElement.Background.Highlight);
		};
	};
};
return t