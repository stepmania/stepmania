return Def.ActorFrame {
	children = {
		LoadActor( "_Tap Receptor", NOTESKIN:LoadActor(Var "Button", "Ready Receptor") ) .. {
			Frame0000=2;
			Delay0000=1;

			InitCommand=cmd(playcommand, "Set");
			GameplayLeadInChangedMessageCommand=cmd(playcommand,"Set");
			SetCommand=cmd(visible,GAMESTATE:GetGameplayLeadIn());
		};

		LoadActor( "_Tap Receptor", NOTESKIN:LoadActor(Var "Button", "Go Receptor") ) .. {
			Frame0000=0;
			Delay0000=0.1;
			Frame0001=1;
			Delay0001=0.8;
			Frame0002=0;
			Delay0002=0.1;
		
			InitCommand=cmd(playcommand, "Set");
			GameplayLeadInChangedMessageCommand=cmd(playcommand,"Set");
			SetCommand=cmd(visible,not GAMESTATE:GetGameplayLeadIn());
		};
	}
}
