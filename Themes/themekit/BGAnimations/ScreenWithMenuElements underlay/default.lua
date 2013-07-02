local t = Def.ActorFrame {
	InitCommand=function(self)
		self:x(SCREEN_CENTER_X);
		self:y(SCREEN_CENTER_Y);
	end;
};
--
t[#t+1] = LoadActor("grid") .. {
	InitCommand=function(self)
		self:zoomto(SCREEN_WIDTH, SCREEN_HEIGHT);
		self:customtexturerect(0, 0, SCREEN_WIDTH / 16, SCREEN_HEIGHT / 16);
	end;
	OnCommand=function(self)
		self:diffuse(color("0,0,0,0.125"));
	end;
};
t[#t+1] = LoadActor("grid") .. {
	InitCommand=function(self)
		self:zoomto(SCREEN_WIDTH, SCREEN_HEIGHT);
		self:customtexturerect(0, 0, SCREEN_WIDTH / 32, SCREEN_HEIGHT / 32);
	end;
	OnCommand=function(self)
		self:diffuse(color("0,0,0,0.25"));
	end;
};
--
return t