return Def.ActorFrame
{
	LoadActor( THEME:GetPathG("", "OptionRowSimple Frame (doubleres)") ) ..
	{
		InitCommand=cmd(x,SCREEN_CENTER_X;y,SCREEN_CENTER_Y+20);
	};
}
