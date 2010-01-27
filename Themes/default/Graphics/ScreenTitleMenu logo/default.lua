-- comments show how this compares to a StepMania 3.9 BGAnimation.ini.
-- It should be noted that additional commands on the ActorFrame as a whole for
-- this file are in metrics.ini ([ScreenTitleMenu] ShowLogo, etc.)
local t = Def.ActorFrame{ -- [BGAnimation]
	LoadActor("_ball"); -- [Layer1] Type=0 File=_ball.png
	LoadActor("_text")..{ -- [Layer2] Type=0 File=_text.png
		InitCommand=cmd(y,48); -- Command=y,48
	};
};

return t;