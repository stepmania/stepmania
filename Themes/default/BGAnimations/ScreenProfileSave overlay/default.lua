local t = Def.ActorFrame {};

t[#t+1] = Def.ActorFrame {
	Def.Sprite {
		Name="animation",
		Texture="LoadScreen diamond 5x2.png",
		InitCommand=function(self)
			self:SetAllStateDelays(0.1):diffusealpha(1):diffuse(color("#981F41")):Center()
		end,
		OnCommand=function(self)
			self:zoom(0):decelerate(0.25):zoom(0.5)
		end;
		OffCommand=function(self)
			self:decelerate(0.25):zoom(0)
		end;
	};
};

t[#t+1] = Def.Actor {
	BeginCommand=function(self)
		if SCREENMAN:GetTopScreen():HaveProfileToSave() then self:sleep(1); end;
		self:queuecommand("Load");
	end;
	LoadCommand=function() SCREENMAN:GetTopScreen():Continue(); end;
};

return t;