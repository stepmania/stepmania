-- This needs to be an ActorFrame because children will want to load the base and merge their own elements into the table.
return Def.ActorFrame {
	LoadActor( THEME:GetPathB('','_standard decoration required'), "LeftFrame", "LeftFrame" );
	LoadActor( THEME:GetPathB('','_standard decoration required'), "RightFrame", "RightFrame" );
	LoadActor( THEME:GetPathB('','_standard decoration required'), "Header", "Header" );
	LoadActor( THEME:GetPathB('','_standard decoration required'), "Footer", "Footer" );
	LoadActor( THEME:GetPathB('','_standard decoration optional'), "StyleIcon", "StyleIcon" );
	LoadActor( THEME:GetPathB('','_standard decoration optional'), "StageFrame", "StageFrame" );
	LoadActor( THEME:GetPathB('','_standard decoration optional'), "StageDisplay", "StageDisplay" );
};
