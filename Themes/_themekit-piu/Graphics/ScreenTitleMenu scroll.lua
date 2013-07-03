local gc = Var("GameCommand");

return Def.ActorFrame {
	InitCommand=function(self)
		local this = self:GetChildren()
		this.ttf:settext(gc:GetText());
		this.ttf:Stroke(color("#000000"));
		this.ttf:diffuse(color("#ffffff"));
	end;
	LoadFont("_arial", "black 20px")..{ Name="ttf" };
};