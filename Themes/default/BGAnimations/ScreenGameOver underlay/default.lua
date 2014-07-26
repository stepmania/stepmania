return Def.ActorFrame {
	InitCommand=cmd(Center),
	LoadActor(THEME:GetPathG("ScreenGameOver","gameover"))..{
		InitCommand=cmd(y,-16;shadowlength,2)
	},
	LoadFont("Common Normal")..{
		Text=ScreenString("Play again soon!"),
		InitCommand=cmd(y,36;shadowlength,1)
	}
}
