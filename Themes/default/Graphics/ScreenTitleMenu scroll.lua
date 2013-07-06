local gc = Var("GameCommand");

return Def.ActorFrame {
	Def.Quad{
		InitCommand=function(self)
			self:zoomto(256, 26);
			self:fadeleft(0.45);
			self:faderight(0.45);
		end;
		OnCommand=function(self)
			self:diffuseshift();
			self:effectcolor1(color("0,0,0,0.5"));
			self:effectcolor2(color("0,0,0,0.5"));
		end;
		GainFocusCommand=function(self)
			self:stoptweening();
			self:decelerate(0.1);
			self:zoomto(256,26);
			self:diffusealpha(1);
		end;
		LoseFocusCommand=function(self)
			self:stoptweening();
			self:accelerate(0.1);
			self:zoomto(SCREEN_WIDTH,0);
			self:diffusealpha(0);
		end;
	};
	LoadFont("Common Normal") .. {
		Text=THEME:GetString("ScreenTitleMenu",gc:GetText());
		OnCommand=function(self)
			self:shadowlength(1);
		end;
		GainFocusCommand=function(self)
			self:stoptweening();
			self:linear(0.1);
			self:zoom(1);
			self:diffuse(color("1,1,1,1"));
		end;
		LoseFocusCommand=function(self)
			self:stoptweening();
			self:linear(0.1);
			self:zoom(0.75);
			self:diffuse(color("0.5,0.5,0.5,1"));
		end;
	};
};