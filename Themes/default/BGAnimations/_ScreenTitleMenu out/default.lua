return Def.ActorFrame {
	InitCommand=function(self)
		self:Center();
	end;
	Def.Quad {
		InitCommand=function(self)
			self:zoomto(SCREEN_WIDTH, SCREEN_HEIGHT);
			self:visible(false);
		end;
		StartTransitioningCommand=function(self)
			self:visible(true);
			self:diffuse(Color("Black"));
			self:sleep(3.5);
			self:diffusealpha(0);
		end;
	};
	-- Sublime
	Def.Quad {
		InitCommand=function(self)
			self:zoomto(SCREEN_WIDTH, 80);
			self:diffuse(Color("Orange"));
			self:visible(false);
		end;
		StartTransitioningCommand=function(self)
			self:visible(true);
			self:decelerate(2);
			self:zoomy(44);
			self:decelerate(0.5);
			self:diffusealpha(0);
		end;
	};
	LoadFont("Common Normal") .. {
		Text="This is only the beginning...";
		InitCommand=function(self)
			self:visible(false);
			self:shadowlength(1);
			self:shadowcolor(BoostColor(Color("Orange"),0.5));
		end;
		StartTransitioningCommand=function(self)
			self:visible(true);
			self:zoom(0.75);
			self:fadeleft(1);
			self:faderight(1);
			self:linear(1);
			self:faderight(0);
			self:fadeleft(0);
			self:sleep(1);
			self:decelerate(0.5);
			self:y(12);
			self:diffusealpha(0);
		end;
	};
	-- End 
	Def.Quad {
		InitCommand=function(self)
			self:zoomto(SCREEN_WIDTH, SCREEN_HEIGHT);
			self:diffuse(Color("White"));
			self:visible(false);
		end;
		StartTransitioningCommand=function(self)
			self:visible(true);
			self:decelerate(1);
			self:diffusealpha(0);
		end;
	};

};