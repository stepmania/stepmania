return LoadActor("bg.png") ..{
	InitCommand=function(self)
		self:x(SCREEN_CENTER_X);
		self:y(SCREEN_CENTER_Y);
		self:zoomtowidth(SCREEN_WIDTH);
		self:zoomtoheight(SCREEN_HEIGHT);
	end;
	OnCommand=function(self)
		self:texcoordvelocity(0, -1);
		self:customtexturerect(0, 0, SCREEN_WIDTH / self:GetWidth(), SCREEN_HEIGHT / self:GetHeight());
		self:diffuse(color("0.9,0.9,0.9,1"));
	end;
};