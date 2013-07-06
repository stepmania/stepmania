local longFail = ThemePrefs.Get("LongFail");

local t = Def.ActorFrame{};

if longFail then
	t[#t+1] = Def.Quad{
		InitCommand=function(self)
			self:FullScreen();
			self:diffuse(color("1,0,0,0"));
			self:blend(Blend.Multiply);
		end;
		OnCommand=function(self)
			self:decelerate(1.25);
			self:diffuse(color("0.75,0,0,0.75"));
			self:linear(7);
			self:diffuse(color("0,0,0,1"));
			self:sleep(1.25);
			self:linear(1);
			self:diffuse(color("1,0,0,1"));
			self:decelerate(2);
			self:diffuse(color("0,0,0,1"));
		end;
	};
	t[#t+1] = Def.Quad{
		InitCommand=function(self)
			self:FullScreen();
			self:diffuse(color("1,1,1,1"));
			self:diffusealpha(0);
		end;
		OnCommand=function(self)
			self:finishtweening();
			self:diffusealpha(1);
			self:decelerate(1.25);
			self:diffuse(color("1,0,0,0"));
		end;
	};
	t[#t+1] = LoadActor(THEME:GetPathS( "ScreenGameplayAlternate", "failed" ) ) .. {
		StartTransitioningCommand=function(self)
			self:play();
		end;
	};
else
	t[#t+1] = Def.Quad{
		InitCommand=function(self)
			self:FullScreen();
			self:diffuse(color("1,0,0,0"));
			self:blend(Blend.Multiply);
		end;
		OnCommand=function(self)
			self:smooth(1);
			self:diffuse(color("0.75,0,0,0.75"));
			self:decelerate(2);
			self:diffuse(color("0,0,0,1"));
		end;
	};
	t[#t+1] = Def.Quad{
		InitCommand=function(self)
			self:FullScreen();
			self:diffuse(color("1,1,1,1"));
			self:diffusealpha(0);
		end;
		OnCommand=function(self)
			self:finishtweening();
			self:diffusealpha(1);
			self:decelerate(1.25);
			self:diffuse(color("1,0,0,0"));
		end;
	};
	t[#t+1] = LoadActor(THEME:GetPathS( Var "LoadingScreen", "failed" ) ) .. {
		StartTransitioningCommand=function(self)
			self:play();
		end;
	};
end;

return t;