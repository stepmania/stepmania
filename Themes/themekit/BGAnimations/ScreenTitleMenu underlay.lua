-- This adds the logo man~
InitUserPrefs();
local t = Def.ActorFrame {};

t[#t+1] = StandardDecorationFromFileOptional("Logo","Logo");

return t