local t = Def.ActorFrame{};

t[#t+1] = Def.ActorFrame {
	InitCommand=cmd(y,48);
	Def.Quad {
		InitCommand=cmd(vertalign,bottom;zoomto,SCREEN_WIDTH,48);
		OnCommand=cmd(diffuse,Color.Black;diffusealpha,0.75);
	};
};

t[#t+1] = Def.ActorFrame {
	InitCommand=cmd(y,48);
	Def.Quad {
		InitCommand=cmd(vertalign,bottom;zoomto,SCREEN_WIDTH,24);
		OnCommand=cmd(diffuse,Color.Orange;diffusealpha,0.125;fadetop,1);
	};
	Def.Quad {
		InitCommand=cmd(vertalign,bottom;zoomto,SCREEN_WIDTH,2);
		OnCommand=cmd(diffuse,Color.Orange);
	};
};

t[#t+1] = Def.ActorFrame {
	Name="MenuTimerDecoration";
	InitCommand=cmd(y,40;visible,false);

	LoadFont("Common Normal") .. {
		Text="TIME";
		InitCommand=cmd(vertalign,bottom;horizalign,right;x,SCREEN_CENTER_X-16;y,-4.5);
		OnCommand=cmd(zoom,0.5);
	};
--[[ 	LoadFont("Common Normal") .. {
		Text="99";
		InitCommand=cmd(vertalign,bottom;horizalign,right;x,SCREEN_CENTER_X-64;y,-4.5);
		OnCommand=cmd();
	}; --]]
};

t[#t+1] = Def.ActorFrame {
	Name="HeaderTextDecoration";
	InitCommand=cmd(y,40);

	LoadFont("Common Normal") .. {
		Text="OPTIONS";
		InitCommand=cmd(vertalign,bottom;horizalign,left;x,-SCREEN_CENTER_X+16;y,-4.5;diffusebottomedge,color("#EEEEEE"));
		OnCommand=cmd();
	};
};

return t
