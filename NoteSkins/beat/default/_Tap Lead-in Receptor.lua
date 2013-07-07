return Def.ActorFrame {
	children = {
		LoadActor( "_Tap Receptor", NOTESKIN:LoadActor(Var "Button", "Ready Receptor") ) .. {
			Frame0000=0;
			Delay0000=1;

			InitCommand=function(self)
				self:playcommand("Set");
			end;
			GameplayLeadInChangedMessageCommand=function(self)
				self:playcommand("Set");
			end;
			SetCommand=function(self)
				self:visible(GAMESTATE:GetGameplayLeadIn());
			end;
		};

		LoadActor( "_Tap Receptor", NOTESKIN:LoadActor(Var "Button", "Go Receptor") ) .. {
			Frame0000=0;
			Delay0000=0;
		
			InitCommand=function(self)
				self:playcommand("Set");
			end;
			GameplayLeadInChangedMessageCommand=function(self)
				self:playcommand("Set");
			end;
			SetCommand=function(self)
				self:visible(not GAMESTATE:GetGameplayLeadIn());
			end;
		};
	}
}
