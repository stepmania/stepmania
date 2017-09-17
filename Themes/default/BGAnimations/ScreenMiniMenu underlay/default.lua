return Def.ActorFrame {
	OnCommand= function(self) self:queuecommand("Recenter") end,
	RecenterCommand= function(self) SCREENMAN:GetTopScreen():xy(0, 0) end,
	Def.Quad{
		InitCommand=cmd(scaletocover,-SCREEN_WIDTH*2,SCREEN_TOP,SCREEN_WIDTH*2,SCREEN_BOTTOM;diffuse,color("0,0,0,0.5"));
	};
};