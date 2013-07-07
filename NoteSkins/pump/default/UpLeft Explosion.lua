return Def.ActorFrame {
	--note graphic
	NOTESKIN:LoadActor(Var "Button", "Tap Note") .. {
		InitCommand=function(self)
			self:blend("BlendMode_Add");
			self:playcommand("Glow");
		end;
		W1Command=function(self)
			self:playcommand("Glow");
		end;
		W2Command=function(self)
			self:playcommand("Glow");
		end;
		W3Command=function(self)
			self:playcommand("Glow");
		end;
		W4Command=function(self) end;
		W5Command=function(self) end;
		HitMineCommand=function(self)
			self:playcommand("Glow");
		end;
		HeldCommand=function(self)
			self:playcommand("Glow");
		end;
		GlowCommand=function(self)
			self:setstate(0);
			self:finishtweening();
			self:diffusealpha(1.0);
			self:zoom(1.0);
			self:linear(0.15);
			self:diffusealpha(0.9);
			self:zoom(1.15);
			self:linear(0.15);
			self:diffusealpha(0.0);
			self:zoom(1.3);
		end;
		
	};
	--tap
	NOTESKIN:LoadActor(Var "Button", "Ready Receptor")..{
		Name="Tap";
		--Frames = { { Frame = 2 ; Delay = 1 } };
		TapCommand=function(self)
			self:finishtweening();
			self:diffusealpha(1);
			self:zoom(1);
			self:linear(0.2);
			self:diffusealpha(0);
			self:zoom(1.2);
		end;
		InitCommand=function(self)
			self:pause();
			self:setstate(2);
			self:playcommand("Tap");
		end;
		HeldCommand=function(self)
			self:playcommand("Tap");
		end;
		ColumnJudgmentMessageCommand=function(self)
			self:playcommand("Tap");
		end;
	};
	--explosion
	LoadActor("_flash")..{
		InitCommand=function(self)
			self:blend("BlendMode_Add");
			self:playcommand("Glow");
		end;
		W1Command=function(self)
			self:playcommand("Glow");
		end;
		W2Command=function(self)
			self:playcommand("Glow");
		end;
		W3Command=function(self)
			self:playcommand("Glow");
		end;
		W4Command=function(self) end;
		W5Command=function(self) end;
		HitMineCommand=function(self)
			self:playcommand("Glow");
		end;
		HeldCommand=function(self)
			self:playcommand("Glow");
		end;
		GlowCommand=function(self)
			self:setstate(0);
			self:finishtweening();
			self:diffusealpha(1);
			self:zoom(1);
			self:linear(0.2);
			self:diffusealpha(0);
			self:zoom(1.2);
		end;
	};
	--thing...
	Def.Quad {
		InitCommand=function(self)
			self:zoomto(50, 5000);
			self:diffusealpha(0);
		end;
		HitMineCommand=function(self)
			self:finishtweening();
			self:diffusealpha(1);
			self:linear(0.3);
			self:diffusealpha(0);
		end;
	};
}