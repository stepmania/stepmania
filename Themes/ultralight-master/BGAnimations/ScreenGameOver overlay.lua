return Def.ActorFrame{
	LoadActor(THEME:GetPathG("","_gameover"))..{
		InitCommand=cmd(Center;diffusealpha,0),
		OnCommand=cmd(accelerate,0.75;diffusealpha,0.75;glow,color("1,1,1,1");decelerate,1;glow,color("1,1,1,0")),
	},
}