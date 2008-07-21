-- This needs to be an ActorFrame because children will want to load the base and merge their own elements into the table.
t =  Def.ActorFrame {};
t[#t+1] = LoadActor( THEME:GetPathB('','_standard decoration required'), "LeftFrame", "LeftFrame" );
t[#t+1] = LoadActor( THEME:GetPathB('','_standard decoration required'), "RightFrame", "RightFrame" );
t[#t+1] = LoadActor( THEME:GetPathB('','_standard decoration required'), "Header", "Header" );
t[#t+1] = LoadActor( THEME:GetPathB('','_standard decoration required'), "Footer", "Footer" );
t[#t+1] = LoadActor( THEME:GetPathB('','_standard decoration optional'), "Help", "Help" );
t[#t+1] = LoadActor( THEME:GetPathB('','_standard decoration optional'), "StyleIcon", "StyleIcon" );
t[#t+1] = LoadActor( THEME:GetPathB('','_standard decoration optional'), "StageFrame", "StageFrame" );
t[#t+1] = LoadActor( THEME:GetPathB('','_standard decoration optional'), "StageDisplay", "StageDisplay" );
return t;

