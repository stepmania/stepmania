return Def.ActorFrame {
	children = {
		LoadActor( "_Tap Receptor", NOTESKIN:LoadActor(Var "Button", "Ready Receptor") ) .. {
			Frame0000=0;
			Delay0000=1;

			InitCommand=cmd(playcommand, "Set");
			GameplayLeadInChangedMessageCommand=cmd(playcommand,"Set");
			SetCommand=cmd(visible,GAMESTATE:GetGameplayLeadIn());
		};

		LoadActor( "_Tap Receptor", NOTESKIN:LoadActor(Var "Button", "Go Receptor") ) .. {
			Frame0000=0;
			Delay0000=1;
		
			InitCommand=cmd(playcommand, "Set");
			GameplayLeadInChangedMessageCommand=cmd(playcommand,"Set");
			SetCommand=cmd(visible,not GAMESTATE:GetGameplayLeadIn());
		};
	}
}
