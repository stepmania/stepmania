local t = Def.ActorFrame {};
--
t[#t+1] = Def.ActorFrame {
	InitCommand=function(self)
		self:x(SCREEN_CENTER_X);
		self:y(SCREEN_CENTER_Y);
	end;
	LoadActor("VOL1-29-NTSC") .. {
		InitCommand=function(self)
			self:scaletoclipped(SCREEN_WIDTH, SCREEN_HEIGHT);
		end;
		OnCommand=function(self)
			self:diffusealpha(0.75);
		end;
	};
};
--
local bShow = 0;
t[#t+1] = Def.ActorFrame {
	InitCommand=function(self)
		self:visible(false);
	end;
	ToggleConsoleDisplayMessageCommand=function(self)
		bShow = 1 - bShow;
		self:visible( bShow == 1 );
	end;
	-- Grid
	LoadActor("_32") .. {
		InitCommand=function(self)
			self:x(SCREEN_CENTER_X);
			self:y(SCREEN_CENTER_Y);
			self:zoomto(SCREEN_WIDTH, SCREEN_HEIGHT);
			self:customtexturerect(0, 0, SCREEN_WIDTH / 32, SCREEN_HEIGHT / 32);
		end;
		OnCommand=function(self)
			self:diffuse(color("0,0,0,0.5"));
		end;
	};
	LoadActor("_16") .. {
		InitCommand=function(self)
			self:x(SCREEN_CENTER_X);
			self:y(SCREEN_CENTER_Y);
			self:zoomto(SCREEN_WIDTH, SCREEN_HEIGHT);
			self:customtexturerect(0, 0, SCREEN_WIDTH / 16, SCREEN_HEIGHT / 16);
		end;
		OnCommand=function(self)
			self:diffuse(color("1,1,1,0.125"));
		end;
	};
	-- Left
	Def.Quad {
		InitCommand=function(self)
			self:horizalign(left);
			self:x(SCREEN_LEFT);
			self:y(SCREEN_CENTER_Y)
			self:zoomto(16, SCREEN_HEIGHT);
		end;
		OnCommand=function(self)
			self:diffuse(color("0,0,0,0.5"));
		end;
	};
	-- Right
	Def.Quad {
		InitCommand=function(self)
			self:horizalign(right);
			self:x(SCREEN_RIGHT);
			self:y(SCREEN_CENTER_Y)
			self:zoomto(16, SCREEN_HEIGHT);
		end;
		OnCommand=function(self)
			self:diffuse(color("0,0,0,0.5"));
		end;
	};
};
--
return t