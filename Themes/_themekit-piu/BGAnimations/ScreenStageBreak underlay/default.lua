return Def.ActorFrame {
	LoadFont("_impact 50px")..{
		Text="Epic fail...";
		InitCommand=function(self)
			self:Center();
			self:diffuse(color("#000000"));
			self:Stroke(color("#ffffff"));
		end;
	};
}