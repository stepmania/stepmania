local t = Def.ActorFrame{
	Font("mentone","24px") .. {
		Text="ultralight";
		InitCommand=cmd(strokecolor,HSVA(0,0,0,0);halign,1;shadowlength,1;shadowcolor,HSVA(192,1,0.5,0.75));
		OnCommand=cmd(queuecommand,"Anim");
		AnimCommand=cmd(linear,1;diffusebottomedge,HSV(192,0.5,0.825));
		OffCommand=cmd(decelerate,0.25;zoomy,0);
	};
	Font("mentone","24px") .. {
		Text="ultralight";
		InitCommand=cmd(strokecolor,HSVA(0,0,0,0);halign,1;shadowlength,0;blend,Blend.Add);
		OnCommand=cmd(queuecommand,"Anim");
		AnimCommand=cmd(bounceend,0.375;zoom,2;diffusealpha,0);
	};
	Font("mentone","24px") .. {
		Text=themeInfo.FriendlyVersion.." / "..themeInfo.Date;
		-- was y,16;
		InitCommand=cmd(x,-38;y,12;zoomx,1024;zoomy,0.45;strokecolor,HSVA(0,0,0,0);halign,1;valign,0;shadowlength,1;shadowcolor,HSVA(192,1,0.5,0.75));
		OnCommand=cmd(queuecommand,"Anim");
		AnimCommand=cmd(accelerate,0.35;diffusebottomedge,HSV(192,0.5,0.9);zoomx,0.45);
		OffCommand=cmd(decelerate,0.125;zoomy,180;sleep,0.002;accelerate,0.125;addy,SCREEN_CENTER_Y*1.5;zoomy,0.01;);
	};
	Font("mentone","24px") .. {
		Text=themeInfo.Version;
		-- was y,16;
		InitCommand=cmd(x,-24;y,12;zoomx,1024;zoomy,0.45;strokecolor,HSVA(0,0,0,0);halign,0;valign,0;shadowlength,1;shadowcolor,HSVA(192,1,0.5,0.75));
		OnCommand=cmd(queuecommand,"Anim");
		AnimCommand=cmd(accelerate,0.35;diffusebottomedge,HSV(192,0.5,0.9);zoomx,0.45);
		OffCommand=cmd(decelerate,0.125;zoomy,180;sleep,0.002;accelerate,0.125;addy,SCREEN_CENTER_Y*1.5;zoomy,0.01;);
	};
};

return t;