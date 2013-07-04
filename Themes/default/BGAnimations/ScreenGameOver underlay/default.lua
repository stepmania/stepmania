return Def.ActorFrame {
	InitCommand=function(self)
		self:x(SCREEN_CENTER_X);
		self:y(SCREEN_CENTER_Y);
	end;
	LoadFont("Common Normal") .. {
		Text=ScreenString("GAME OVER");
		InitCommand=function(self)
			self:y(-4);
			self:shadowlength(1);
			self:diffuse(Color("Red"));
		end;
	};
	LoadFont("Common Normal") .. {
		Text=ScreenString("Play again soon!");
		InitCommand=function(self)
			self:y(16);
			self:shadowlength(1);
			self:zoom(0.5);
		end;
	};
};