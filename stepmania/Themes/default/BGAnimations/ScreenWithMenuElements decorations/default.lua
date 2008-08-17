-- This needs to be an ActorFrame because children will want to load the base and merge their own elements into the table.
t =  Def.ActorFrame {};
t[#t+1] = StandardDecorationFromFile( "LeftFrame", "LeftFrame" );
t[#t+1] = StandardDecorationFromFile( "RightFrame", "RightFrame" );
t[#t+1] = StandardDecorationFromFile( "Header", "Header" );
t[#t+1] = StandardDecorationFromFile( "Footer", "Footer" );
t[#t+1] = StandardDecorationFromFileOptional( "Help", "Help" );
t[#t+1] = StandardDecorationFromFileOptional( "StyleIcon", "StyleIcon" );
t[#t+1] = StandardDecorationFromFileOptional( "StageFrame", "StageFrame" );
t[#t+1] = StandardDecorationFromFileOptional( "StageDisplay", "StageDisplay" );
return t;

