-- how does installed song??? let's find out

local t = Def.ActorFrame{
	LoadFont("Common Normal")..{
		Name="Header";
		InitCommand=cmd(x,SCREEN_LEFT+24;y,SCREEN_TOP+24;halign,0;diffuse,color("#CCCCCC");settext,Screen.String("BodyHeader");shadowlength,1;shadowcolor,HSV(40,0,0.6);diffusetopedge,color("#FFFFFF"));
		OnCommand=cmd(queuecommand,"Anim");
		AnimCommand=cmd(cropright,1;faderight,1;addx,96;decelerate,1;addx,-96;skewx,-0.1;cropright,0;faderight,0;);
	};
	-- todo: add explantion paragraph here (above the scroller)
};

return t;