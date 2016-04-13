local t = Def.ActorFrame{
	Def.Quad{
		Name="TopLine";
		InitCommand=cmd(x,SCREEN_CENTER_X;y,SCREEN_CENTER_Y*0.7;addy,-SCREEN_CENTER_Y;zoomto,SCREEN_WIDTH,2;diffuseleftedge,HSV(192,1,0.8););
		OnCommand=cmd(linear,0.3;addy,SCREEN_CENTER_Y);
		OffCommand=cmd(linear,0.3;addy,-SCREEN_CENTER_Y);
		CancelCommand=cmd(linear,0.3;diffuserightedge,HSV(192,1,0.8);diffuseleftedge,HSV(192,0,1););
	};
	Font("mentone","24px") .. {
		Text=HeaderString("ThemeOptions");
		InitCommand=cmd(x,SCREEN_LEFT+32;y,SCREEN_CENTER_Y*0.68;addy,-SCREEN_CENTER_Y;zoom,0.8;halign,0;valign,1;shadowlength,1;strokecolor,color("0,0,0,0"));
		OnCommand=cmd(accelerate,0.2999;diffusebottomedge,HSV(192,0.2,0.8);addy,SCREEN_CENTER_Y;);
		OffCommand=cmd(linear,0.3;addy,-SCREEN_CENTER_Y);
		CancelCommand=cmd(bouncebegin,0.3;zoomy,0;);
	};
	Def.Quad{
		Name="BottomLine";
		InitCommand=cmd(x,SCREEN_CENTER_X;y,SCREEN_CENTER_Y*1.2;addy,SCREEN_CENTER_Y;zoomto,SCREEN_WIDTH,2;diffuserightedge,HSV(192,1,0.8););
		OnCommand=cmd(linear,0.3;addy,-SCREEN_CENTER_Y);
		OffCommand=cmd(linear,0.3;addy,SCREEN_CENTER_Y);
		CancelCommand=cmd(linear,0.3;diffuseleftedge,HSV(192,1,0.8);diffuserightedge,HSV(192,0,1););
	};
};

return t;