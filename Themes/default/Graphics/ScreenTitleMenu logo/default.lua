-- comments show how this compares to a StepMania 3.9 BGAnimation.ini.
-- It should be noted that additional commands on the ActorFrame as a whole for
-- this file are in metrics.ini ([ScreenTitleMenu] ShowLogo, etc.)
local t = Def.ActorFrame{ -- [BGAnimation]
	LoadActor("_logo"); -- [Layer1] Type=0 File=_ball.png
	LoadActor("_logo")..{ -- [Layer2] Type=0 File=_ball.png
	};
	--[[LoadActor("_text")..{ -- [Layer3] Type=0 File=_text.png
		InitCommand=cmd(y,48;hide_if,GAMESTATE:GetMultiplayer()); -- Command=y,48
	};
	LoadActor("_multi")..{ -- [Layer4] Type=0 File=_multi.png
		InitCommand=cmd(y,60;hide_if,not GAMESTATE:GetMultiplayer()); -- Command=y,48
		-- the hide_if command would be similar to Condition=, which also exists.
		-- Condition=GAMESTATE:GetMultiplayer(); is the equivalent code.
	};]]
	LoadFont("Common normal")..{
		Text="sm-ssc Multiplayer";
		InitCommand=cmd(y,5;shadowlength,0;strokecolor,color("0,0,0,0.375");diffusebottomedge,color("#D6DBDD"););
		BeginCommand=cmd(hide_if,not GAMESTATE:GetMultiplayer());
	};
};

return t;
