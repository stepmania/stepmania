local t = Def.ActorFrame {};
--
--[[ local tGridSizeMajor = { 32, 32 };
local tGridSizeMinor = { 16, 16 };
local function CreateDebugGrid()
	local t = Def.ActorFrame {};
	local numX = math.ceil(SCREEN_WIDTH/tGridSizeMajor[1])
	local numY = math.ceil(SCREEN_HEIGHT/tGridSizeMajor[2])
	local offset = ( math.ceil(SCREEN_WIDTH/tGridSizeMajor[1]) - SCREEN_WIDTH ) + SCREEN_WIDTH;
	for a=1,numY do
		for b=1,numX do 
			t[#t+1] = Def.Quad {
				InitCommand=function(self)
					self:x(b * tGridSizeMajor[1] - (tGridSizeMajor[1] / 2));
					self:y(a * tGridSizeMajor[2] - (tGridSizeMajor[2] / 2));
				end;
				OnCommand=function(self)
					self:zoomto(tGridSizeMajor[1] - 2, tGridSizeMajor[2] - 2);
					self:diffuse(color("1,1,1,0.25"));
				end;
			};
		end;
	end;
	return t
end --]]

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
	InitCommand=funtion(self)
		self:visible(false);
	end;
	ToggleConsoleDisplayMessageCommand=function(self)
		bShow = 1 - bShow;
		self:visible( bShow == 1 );
	end;
	-- Grid
--[[ 	
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
--]]
--[[ 	
	LoadActor("_8") .. {
		InitCommand=function(self)
			self:x(SCREEN_CENTER_X);
			self:y(SCREEN_CENTER_Y);
			self:zoomto(SCREEN_WIDTH, SCREEN_HEIGHT);
			self:customtexturerect(0, 0, SCREEN_WIDTH / 8, SCREEN_HEIGHT / 8);
		end;
		OnCommand=function(self)
			self:diffuse(color("#00BFE833"));
		end;
	};
--]]
	-- Left
	Def.Quad {
		InitCommand=function(self)
			self:horizalign(left);
			self:x(SCREEN_LEFT);
			self:y(SCREEN_CENTER_Y);
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
			self:y(SCREEN_CENTER_Y);
			self:zoomto(16, SCREEN_HEIGHT);
		end;
		OnCommand=function(self)
			self:diffuse(color("0,0,0,0.5"));
		end;
	};
};
--
return t