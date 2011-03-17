-- ScreenWithMenuElements decorations is a very important file, as it will put a
-- decorations layer on pretty much every screen.
local t = Def.ActorFrame{};

t[#t+1] = StandardDecorationFromFileOptional("Header","Header");
--t[#t+1] = StandardDecorationFromFileOptional("Footer","Footer");

return t;