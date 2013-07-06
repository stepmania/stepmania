local gc = Var "GameCommand";
local colors = {
	Easy		= color("#00ff00"),
	Normal		= color("#feee00"),
	Hard		= color("#feee00"),
	Rave		= color("#db93ff"),
	Nonstop		= color("#00ffff"),
	Oni			= color("#d70b8c"),
	Endless		= color("#b4c3d2"),
};
local t = Def.ActorFrame {};
-- Background!
t[#t+1] = Def.ActorFrame {
	LoadActor(THEME:GetPathG("ScreenSelectPlayMode","BackgroundFrame")) .. {
		InitCommand=function(self)
			self:diffuse(Color("Black"));
			self:diffusealpha(0.7);
		end;
		GainFocusCommand=function(self)
			self:visible(true);
		end;
		LoseFocusCommand=function(self)
			self:visible(false);
		end;
	};
 	LoadActor("_HighlightFrame") .. {
		InitCommand=function(self)
			self:diffuse(ModeIconColors[gc:GetName()]);
			self:diffusealpha(0);
		end;
		GainFocusCommand=function(self)
			self:finishtweening();
			self:diffusealpha(1);
			self:glow(Color.Alpha(Color.White,1));
			self:linear(0.1);
			self:glow(Color.Invisible);
		end;
		LoseFocusCommand=function(self)
			self:finishtweening();
			self:diffusealpha(0);
			self:glow(Color.Invisible);
		end;
		OffFocusedCommand=function(self)
			self:finishtweening();
			self:glow(Color("White"));
			self:decelerate(1);
			self:glow(Color("Invisible"));
		end;
	};
};
-- Emblem Frame
t[#t+1] = Def.ActorFrame {
	FOV=90;
	InitCommand=function(self)
		self:x(-192);
		self:zoom(0.9);
	end;
	-- Main Shadow
	LoadActor( gc:GetName() ) .. {
		InitCommand=function(self)
			self:x(2);
			self:y(2);
			self:diffuse(Color("Black"));
			self:diffusealpha(0);
			self:zoom(0.75);
		end;
		GainFocusCommand=function(self)
			self:stoptweening();
			self:stopeffect();
			self:smooth(0.1);
			self:diffusealpha(0);
			self:zoom(1);
			self:decelerate(0.05);
			self:diffusealpha(0.5);
			self:pulse();
			self:effecttiming(0.75, 0.125, 0.125, 0.75);
			self:effectmagnitude(0.95, 1, 1);
		end;
		LoseFocusCommand=function(self)
			self:stoptweening();
			self:stopeffect();
			self:smooth(0.2);
			self:diffusealpha(0);
			self:zoom(0.75);
		end;
		OffFocusedCommand=function(self)
			self:finishtweening();
			self:stopeffect();
			self:glow(ModeIconColors[gc:GetName()]);
			self:decelerate(0.5);
			self:rotationy(360);
		end;
	};
	-- Main Emblem
	LoadActor( gc:GetName() ) .. {
		InitCommand=function(self)
			self:diffusealpha(0);
			self:zoom(0.75);
		end;
		GainFocusCommand=function(self)
			self:stoptweening();
			self:stopeffect();
			self:smooth(0.1);
			self:diffusealpha(1);
			self:zoom(1);
			self:glow(Color("White"));
			self:decelerate(0.05);
			self:glow(Color("Invisible"));
			self:pulse();
			self:effecttiming(0.75, 0.125, 0.125, 0.75);
			self:effectmagnitude(0.95, 1, 1);
		end;
		LoseFocusCommand=function(self)
			self:stoptweening();
			self:stopeffect();
			self:smooth(0.2);
			self:diffusealpha(0);
			self:zoom(0.75);
			self:glow(Color("Invisible"));
		end;
		OffFocusedCommand=function(self)
			self:finishtweening();
			self:stopeffect();
			self:glow(ModeIconColors[gc:GetName()]);
			self:decelerate(0.5);
			self:rotationy(360);
			self:glow(Color("Invisible"));
		end;
	};
};
-- Text Frame
t[#t+1] = Def.ActorFrame {
	InitCommand=function(self)
		self:x(-192/2);
		self:y(-10);
	end;
	Def.Quad {
		InitCommand=function(self)
			self:horizalign(left);
			self:y(20);
			self:zoomto(320, 2);
			self:diffuse(ModeIconColors[gc:GetName()]);
			self:diffusealpha(0);
			self:fadeleft(0.35);
			self:faderight(0.35);
		end;
		GainFocusCommand=function(self)
			self:stoptweening();
			self:linear(0.1);
			self:diffusealpha(1);
		end;
		LoseFocusCommand=function(self)
			self:stoptweening();
			self:linear(0.1);
			self:diffusealpha(0);
		end;
	};
	LoadFont("_helveticaneuelt std extblk cn 42px") .. {
		Text=gc:GetText();
		InitCommand=function(self)
			self:horizalign(left);
			self:diffuse(ModeIconColors[gc:GetName()]);
			self:shadowcolor(ColorDarkTone(ModeIconColors[gc:GetName()]));
			self:shadowlength(2);
			self:diffusealpha(0);
			self:skewx(-0.125);
		end;
		GainFocusCommand=function(self)
			self:stoptweening();
			self:x(-32);
			self:decelerate(0.1);
			self:diffusealpha(1);
			self:x(0);
		end;
		LoseFocusCommand=function(self)
			self:stoptweening();
			self:x(0);
			self:accelerate(0.1);
			self:diffusealpha(0);
			self:x(32);
			self:diffusealpha(0);
		end;
	};
	LoadFont("_helveticaneuelt std extblk cn 42px") .. {
		Text=THEME:GetString(Var "LoadingScreen", gc:GetName() .. "Explanation");
		InitCommand=function(self)
			self:horizalign(right);
			self:x(320);
			self:y(30);
			self:shadowlength(1);
			self:diffusealpha(0);
			self:skewx(-0.125);
			self:zoom(0.5);
		end;
		GainFocusCommand=function(self)
			self:stoptweening();
			self:x(320+32);
			self:decelerate(0.1);
			self:diffusealpha(1);
			self:x(320);
		end;
		LoseFocusCommand=function(self)
			self:stoptweening();
			self:x(320);
			self:accelerate(0.1);
			self:diffusealpha(0);
			self:x(320-32);
			self:diffusealpha(0);
		end;
	};
};
t.GainFocusCommand=function(self)
	self:finishtweening();
	self:visible(true);
	self:zoom(1.1);
	self:decelerate(0.25);
	self:zoom(1);
end;
t.LoseFocusCommand=function(self)
	self:finishtweening();
	self:visible(false);
	self:zoom(1);
end;
return t