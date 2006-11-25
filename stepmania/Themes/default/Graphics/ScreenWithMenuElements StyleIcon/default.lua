local style = GAMESTATE:GetCurrentStyle();
if not style then return Def.Actor {}; end

local s = style:GetStyleType();
local Reverse = PlayerNumber:Reverse();
s = string.gsub(s, "StyleType_", "");

local t = LoadActor("_icon " .. s)() ..  {
	InitCommand = cmd(pause;setstate,Reverse[GAMESTATE:GetMasterPlayerNumber()]);
};

return t;
