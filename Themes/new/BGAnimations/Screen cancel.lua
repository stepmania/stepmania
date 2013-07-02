local t = Def.ActorFrame {};
--
t[#t+1] = Def.ActorFrame {
	InitCommand=function(self)
		self:Center();
	end;
	--
	Def.Quad {
		InitCommand=function(self)
			self:zoomto(SCREEN_WIDTH, SCREEN_HEIGHT);
			self:diffuse(Color.Black);
			self:diffusealpha(0);
		end;
		StartTransitioningCommand=function(self)
			self:decelerate(0.125);
			self:diffusealpha(1);
		end;
	};
	-- Sounds
	LoadActor(THEME:GetPathS(Var "LoadingScreen","cancel")) .. {
		StartTransitioningCommand=function(self)
			self:play();
		end;
	};
};
--
return t