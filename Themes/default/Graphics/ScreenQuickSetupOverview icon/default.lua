local gc = Var "GameCommand";
local c = {};
	c.X = THEME:GetMetric( Var "LoadingScreen", "Icon" .. gc:GetName() .. "X");
	c.Y = THEME:GetMetric( Var "LoadingScreen", "Icon" .. gc:GetName() .. "Y");
local t = Def.ActorFrame {};
t[#t+1] = Def.ActorFrame {
	Condition=( gc:GetName() ~= "Back" );
	InitCommand=function(self)
		self:x(c.X);
		self:y(c.Y);
	end;
	GainFocusCommand=function(self)
		self:finishtweening();
		self:zoom(1.125);
		self:bounceend(0.125);
		self:zoom(1);
	end;
	LoseFocusCommand=function(self)
		self:stoptweening();
		self:linear(0.125);
		self:zoom(0.875);
	LoadActor("_base") .. {
		GainFocusCommand=function(self)
			self:stoptweening();
			self:linear(0.125);
			self:diffuse(Color("Orange"));
			self:diffusetopedge(Color("Yellow"));
		end;
		LoseFocusCommand=function(self)
			self:stoptweening();
			self:linear(0.125);
			self:diffuse(Color("White"));
		end;
	};
	LoadFont("Common Normal") .. {
		Text=gc:GetName();
		InitCommand=function(self)
			self:strokecolor(Color("White"));
		end;
		OnCommand=function(self)
			self:diffuse(Color("Black"));
		end;
	};
};
t[#t+1] = Def.ActorFrame {
	Condition=( gc:GetName() == "Back" );
	InitCommand=function(self)
		self:x(c.X);
		self:y(c.Y);
	end;
	GainFocusCommand=function(self)
		self:finishtweening();
		self:zoom(1.125);
		self:bounceend(0.125);
		self:zoom(1);
	end;
	LoseFocusCommand=function(self)
		self:stoptweening();
		self:linear(0.125);
		self:zoom(0.875);
	end;
	LoadActor("_base") .. {
		GainFocusCommand=function(self)
			self:stoptweening();
			self:linear(0.125);
			self:diffuse(Color("Red"));
		end;
		LoseFocusCommand=function(self)
			self:stoptweening();
			self:linear(0.125);
			self:diffuse(Color("White"));
		end;
	};
	LoadFont("Common Normal") .. {
		Text=gc:GetName();
		InitCommand=function(self)
			self:strokecolor(Color("White"));
		end;
		OnCommand=function(self)
			self:diffuse(Color("Black"));
		end;
	};
};

return t;