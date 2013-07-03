return LoadFont("Common", "Normal")..{
	Text="EXIT";
	InitCommand=function(self)
		self:CenterX();
	end;
	GainFocusCommand=function(self)
		self:diffuse(color("#FF0000"));
	end;
	LoseFocusCommand=function(self)
		self:diffuse(color("#FFFFFF"));
	end;
}