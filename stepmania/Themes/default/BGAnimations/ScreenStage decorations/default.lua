local t = Def.ActorFrame {};
local InitCommand=cmd(x,SCREEN_CENTER_X;y,SCREEN_CENTER_Y;);
local pm = GAMESTATE:GetPlayMode();
-- This relies on play modes and stages having the same naming for
-- everything but Regular.
local stage = pm == 'PlayMode_Regular' and GAMESTATE:GetCurrentStage() or 
	'Stage_' .. string.sub(pm, 10);
t[#t+1] = LoadActor( stage ) .. {
	InitCommand=InitCommand;
	OnCommand=cmd(cropbottom,1;linear,0.5;cropbottom,0;sleep,10;);
};
return t;
