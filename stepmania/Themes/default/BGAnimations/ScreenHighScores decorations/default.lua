local t = Def.ActorFrame {
	LoadActor("mockup") .. {
		InitCommand=cmd(x,SCREEN_CENTER_X;y,SCREEN_CENTER_Y;diffusealpha,0.2;);
	};
	Def.ActorFrame {
		InitCommand=cmd(x,SCREEN_CENTER_X;y,SCREEN_CENTER_Y-200;);
		LoadActor("difficulty pill") .. {
			InitCommand=cmd(x,-100;);
		};
		LoadActor("difficulty pill") .. {
			InitCommand=cmd(x,-50;);
		};
		LoadActor("difficulty pill") .. {
			InitCommand=cmd(x,0;);
		};
		LoadActor("difficulty pill") .. {
			InitCommand=cmd(x,50;);
		};
		LoadActor("difficulty pill") .. {
			InitCommand=cmd(x,100;);
		};
	};
}

return t;