local style = GAMESTATE:GetCurrentStyle();
if not style then return Def.Actor {}; end

local s = style:GetName();
local Reverse = PlayerNumber:Reverse();

local t = LoadActor("_icon " .. s)() ..  {
	InitCommand = cmd(pause;setstate,Reverse[GAMESTATE:GetMasterPlayerNumber()]);
};

return t;
