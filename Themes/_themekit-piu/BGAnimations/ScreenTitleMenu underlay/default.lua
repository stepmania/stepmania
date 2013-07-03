return Def.ActorFrame {
	LoadFont("_arial black 20px")..{
		Text="Background logo or video must go here...";
		InitCommand=function(self)
			self:CenterX();
			self:y(200);
			self:Stroke(color("#000000"));
		end;
	};
}