local gc = Var("GameCommand");
local squareSize = 8; -- was 18

return Def.ActorFrame {
	Def.Quad{
		InitCommand=function(self)
			self:x(-12);
			self:zoom(squareSize);
			self:rotationz(45);
			self:diffuse(color("#222222"));
		end;
		GainFocusCommand=function(self)
			self:stoptweening();
			self:accelerate(0.25);
			self:zoom(squareSize);
			self:rotationz(45);
		end;
		LoseFocusCommand=function(self)
			self:stoptweening();
			self:decelerate(0.25);
			self:zoom(0);
			self:rotationz(360+45);
		end;
	};
	LoadFont("Common Normal") .. {
		Text=gc:GetText();
		InitCommand=function(self)
			self:halign(0);
			self:zoom(0.625);
		end;
		GainFocusCommand=function(self)
			self:stoptweening();
			self:decelerate(0.25);
			self:diffuse(color("1,1,1,1"));
		end;
		LoseFocusCommand=function(self)
			self:stoptweening();
			self:accelerate(0.25);
			self:diffuse(color("0.5,0.5,0.5,1"));
		end;
	};
};