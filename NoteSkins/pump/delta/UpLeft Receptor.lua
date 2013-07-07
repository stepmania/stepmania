return Def.ActorFrame {

	NOTESKIN:LoadActor(Var "Button", "Ready Receptor")..{
		Name="Base";
		InitCommand=function(self)
			self:animate(0);
			self:setstate(0);
		end;
	};

	NOTESKIN:LoadActor(Var "Button", "Ready Receptor")..{
		Name="Glow";
		InitCommand=function(self)
			self:animate(0);
			self:setstate(1);
		end;
		OnCommand=function(self)
			self:effectclock("bgm");
			self:diffuseshift();
			self:effectcolor1(color("#FFFFFFFF"));
			self:effectcolor2(color("#FFFFFF00"));
			self:effecttiming(1, 0, 0, 0);
		end;
	};

	NOTESKIN:LoadActor(Var "Button", "Ready Receptor")..{
		Name="Tap";
		InitCommand=function(self)
			self:animate(0);
			self:setstate(2);
			self:zoom(1);
			self:diffusealpha(0);
			self:blend('BlendMode_Add');
		end;
		PressCommand=function(self)
			self:diffuse(color("#FFFFAA"));
			self:stoptweening();
			self:zoom(1.1);
			self:linear(0.1);
			self:diffusealpha(0.6);
			self:zoom(1);
		end;
		LiftCommand=function(self)
			self:diffuse(color("#FFFFAA"));
			self:stoptweening();
			self:diffusealpha(0.6);
			self:zoom(1);
			self:linear(0.15);
			self:zoom(1.2);
			self:diffusealpha(0);
		end;
	};
}