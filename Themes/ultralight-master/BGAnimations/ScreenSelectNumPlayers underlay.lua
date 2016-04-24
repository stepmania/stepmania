local t = Def.ActorFrame{
	Def.Quad{
		Name="TopLine";
		InitCommand=cmd(x,SCREEN_CENTER_X;y,SCREEN_CENTER_Y*0.8;zoomto,SCREEN_WIDTH,2;diffuserightedge,HSV(192,1,0.8););
		OffCommand=cmd(linear,0.375;cropright,1);
		CancelCommand=cmd(linear,0.3;diffuserightedge,HSV(192,1,0.8);diffuseleftedge,HSV(192,0,1);cropleft,1);
	};
	Font("mentone","24px") .. {
		Text=HeaderString("SelectNumPlayers");
		InitCommand=cmd(x,SCREEN_LEFT+32;y,SCREEN_CENTER_Y*0.79;zoomx,0.8;halign,0;valign,1;shadowlength,1;zoomy,0;strokecolor,color("0,0,0,0"));
		OnCommand=cmd(bounceend,0.5;zoomy,0.8;diffusebottomedge,HSV(192,0.2,0.8));
		OffCommand=cmd(linear,0.3;faderight,1;cropright,1);
		CancelCommand=cmd(bouncebegin,0.3;zoomy,0;);
	};
	Def.Quad{
		Name="BottomLine";
		InitCommand=cmd(x,SCREEN_CENTER_X;y,SCREEN_CENTER_Y*1.2;zoomto,SCREEN_WIDTH,2;diffuseleftedge,HSV(192,1,0.8););
		OffCommand=cmd(linear,0.375;cropleft,1);
		CancelCommand=cmd(linear,0.3;diffuseleftedge,HSV(192,1,0.8);diffuserightedge,HSV(192,0,1);cropright,1);
	};
};

return t;