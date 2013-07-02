local gc = Var("GameCommand");
--
local t = Def.ActorFrame {};
--
t[#t+1] = Def.ActorFrame { 
	Def.Quad {
		InitCommand=function(self)
			self:zoomto(96, 24);
			self:diffuse(Color.White);
		end;
		GainFocusCommand=function(self)
			self:diffuseshift();
			self:effectcolor2(Color.White);
			self:effectcolor1(Color.Blue);
		end;
		LoseFocusCommand=function(self)
			self:stopeffect();
		end;
		DisabledCommand=function(self)
			self:stopeffect();
			self:diffuse(color("0.5,0.5,0.5,1"));
		end;
		EnabledCommand=function(self)
			self:stopeffect();
			self:diffuse(Color.White);
		end;
	};
	--
	LoadFont("Common Normal") .. {
		Text=gc:GetText();
		InitCommand=function(self)
			self:diffuse(Color.Black);
			self:zoom(0.675);
		end;
	};
};
return t