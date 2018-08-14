local t = Def.ActorFrame{
	InitCommand=cmd(fov,70);
	Def.ActorFrame {
		InitCommand=cmd(zoom,0.75);
		OnCommand=cmd(diffusealpha,0;zoom,0.4;decelerate,0.7;diffusealpha,1;zoom,0.75);
			LoadActor("_text");
			LoadActor("_text")..{
				Name="TextGlow";
				InitCommand=cmd(blend,Blend.Add;diffusealpha,0.05);
				OnCommand=cmd(glowshift;effectperiod,5;effectcolor1,color("1,1,1,0.25");effectcolor2,color("1,1,1,1"));
			};
		};
	};

return t;
