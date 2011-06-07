-- comments show how this compares to a StepMania 3.9 BGAnimation.ini.
-- It should be noted that additional commands on the ActorFrame as a whole for
-- this file are in metrics.ini ([ScreenTitleMenu] ShowLogo, etc.)
local t = Def.ActorFrame{ -- [BGAnimation]
	LoadActor("_logo"); -- [Layer1] Type=0 File=_ball.png
};

return t;
