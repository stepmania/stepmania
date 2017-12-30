local t = Def.ActorFrame {
	Def.Sprite {
		Texture="_arrow";
		Frame0000=0;
		Delay0000=1;
	};
	Def.Sprite {
		Texture="_circle";
		Frame0000=0;
		Delay0000=1;
		InitCommand=cmd(y,15;effectclock,"beat";diffuseramp;effectcolor1,color("1,1,1,0.1");effectcolor2,color("1,1,1,0.45");effectoffset,0;zoom,0.5);
	};
	Def.Sprite {
		Texture="_circle";
		Frame0000=0;
		Delay0000=1;
		InitCommand=cmd(y,5;effectclock,"beat";diffuseramp;effectcolor1,color("1,1,1,0.1");effectcolor2,color("1,1,1,0.45");effectoffset,0.25;zoom,0.5);
	};
	Def.Sprite {
		Texture="_circle";
		Frame0000=0;
		Delay0000=1;
		InitCommand=cmd(y,-5;effectclock,"beat";diffuseramp;effectcolor1,color("1,1,1,0.1");effectcolor2,color("1,1,1,0.45");effectoffset,0.5;zoom,0.5);
	};
	Def.Sprite {
		Texture="_circle";
		Frame0000=0;
		Delay0000=1;
		InitCommand=cmd(y,-15;effectclock,"beat";diffuseramp;effectcolor1,color("1,1,1,0.1");effectcolor2,color("1,1,1,0.45");effectoffset,0.75;zoom,0.5);
	};
};
return t;
