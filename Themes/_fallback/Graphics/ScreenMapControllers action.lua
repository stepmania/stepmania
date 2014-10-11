return Def.BitmapText{
	Font="Common Normal",
	InitCommand= cmd(x, SCREEN_CENTER_X; zoom, .75, 0; diffuse, color("#808080")),
	OnCommand= function(self)
		self:diffusealpha(0)
		self:decelerate(0.5)
		self:diffusealpha(1)
		-- fancy effect:  Look at the name (which is set by the screen) to set text
		self:settext(
			THEME:GetString("ScreenMapControllers", "Action" .. self:GetName()))
	end,
	OffCommand=cmd(stoptweening;accelerate,0.3;diffusealpha,0;queuecommand,"Hide"),
	HideCommand=cmd(visible,false),
	GainFocusCommand=cmd(diffuseshift;effectcolor2,color("#808080");effectcolor1,color("#FFFFFF")),
	LoseFocusCommand=cmd(stopeffect),
}
