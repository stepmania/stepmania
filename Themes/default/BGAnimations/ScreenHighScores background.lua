return Def.ActorFrame {
	LoadActor(THEMEMAN:GetPathG("common bg", "base")) .. {
		InitCommand=cmd(Center;zoomto,SCREEN_WIDTH,SCREEN_HEIGHT)
	};
}
