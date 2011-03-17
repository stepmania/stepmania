local t = LoadFallbackB();

t[#t+1] = StandardDecorationFromFileOptional("Logo","Logo");
t[#t+1] = LoadActor( THEME:GetPathB("_Arcade","decorations") );

return t;