local gc = Var("GameCommand");

return Def.ActorFrame {
	InitCommand=function(self)
		local this = self:GetChildren()
		this.ttf:settext(gc:GetText());
		this.ttf:Stroke(color("#000000"));
		this.ttf:diffuse(color("#ffffff"));
		--this.bkg:zoomto(150,40);
		--this.bkg:diffuse(color("#c0c0c0"));
		--this.bkg:fadeleft(0.2);
		--this.bkg:faderight(0.2);
	end;
	--GainFocusCommand=cmd(stoptweening;zoom,1.2);
	--LoseFocusCommand=cmd(stoptweening;zoom,1.0);
	--Def.Quad { Name="bkg" };
	LoadFont("_arial", "black 20px")..{ Name="ttf" };
};