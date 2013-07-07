return Def.ActorFrame {

	NOTESKIN:LoadActor(Var "Button","NoteHit") .. {
		InitCommand=function(self)
			self:animate(0);
			self:blend(Blend.Add);
			self:diffusealpha(0);
		end;

		NoneCommand=function(self)
			self:playcommand("Glow");
		end;
		PressCommand=function(self)
			self:playcommand("Glow");
		end;
		W1Command=function(self)
			self:setstate(0);
			self:playcommand("W2");
		end;
		W2Command=function(self)
			self:setstate(0);
			self:playcommand("Glow");
		end;
		W3Command=function(self)
			self:setstate(1);
			self:playcommand("Glow");
		end;
		W4Command=function(self)
			self:setstate(2);
			self:playcommand("Glow");
		end;
		W5Command=function(self) end;
		HitMineCommand=function(self)
			self:playcommand("Glow");
		end;
		HeldCommand=function(self)
			self:setstate(0);
			self:playcommand("Glow");
		end;
		GlowCommand=function(self)
			self:stoptweening();
			self:zoom(1.05);
			self:diffusealpha(1);
			self:linear(0.25);
			self:zoom(1.1);
			self:diffusealpha(0);
		end;
	};

	NOTESKIN:LoadActor(Var "Button","NoteHit") .. {
		InitCommand=function(self)
			self:animate(0);
			self:blend(Blend.Add);
			self:diffusealpha(0);
		end;

		NoneCommand=function(self)
			self:playcommand("Glow");
		end;
		PressCommand=function(self)
			self:playcommand("Glow");
		end;
		W1Command=function(self)
			self:setstate(0);
			self:playcommand("W2");
		end;
		W2Command=function(self)
			self:setstate(0);
			self:playcommand("Glow");
		end;
		W3Command=function(self)
			self:setstate(1);
			self:playcommand("Glow");
		end;
		W4Command=function(self)
			self:setstate(2);
			self:playcommand("Glow");
		end;
		W5Command=function(self) end;
		HitMineCommand=function(self)
			self:playcommand("Glow");
		end;
		HeldCommand=function(self)
			self:setstate(0);
			self:playcommand("Glow");
		end;
		GlowCommand=function(self)
			self:stoptweening();
			self:zoom(1);
			self:diffusealpha(0.4);
			self:linear(0.3);
			self:zoom(1.2);
			self:diffusealpha(0);
		end;
	};

	NOTESKIN:LoadActor(Var "Button","NoteHit") .. {
		InitCommand=function(self)
			self:animate(0);
			self:zoom(1.1);
			self:blend(Blend.Add);
			self:visible(false);
		end;
		HoldingOnCommand=function(self)
			self:visible(true);
		end;
		HoldingOffCommand=function(self)
			self:visible(false);
		end;
	};


}