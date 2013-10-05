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
				InitCommand=cmd(x,b*tGridSizeMajor[1]-(tGridSizeMajor[1]/2);y,a*tGridSizeMajor[2]-(tGridSizeMajor[2]/2));
				OnCommand=cmd(zoomto,tGridSizeMajor[1]-2,tGridSizeMajor[2]-2;diffuse,color("1,1,1,0.25"));
			};
		end;
	end;
	return t
end --]]

--
t[#t+1] = Def.ActorFrame {
	Def.Quad {
		InitCommand=cmd(FullScreen;diffuse,color("#1C1C1C"));
	};
	LoadActor( THEME:GetPathG("_texture","checkerboard") ) .. {
		InitCommand=cmd(FullScreen;
						customtexturerect,0,0,SCREEN_WIDTH/64,SCREEN_HEIGHT/64;
						Center);
		OnCommand=cmd(texcoordvelocity,0.25,0;diffusealpha,0.025);
	};
	Def.Quad {
		InitCommand=cmd(FullScreen;diffuse,Color.Orange;diffusealpha,0.6);
		OnCommand=cmd(fadeleft,1;cropleft,0.45;diffuseshift;effectperiod,5;
					  effectcolor2,Color.Alpha(Color.Orange,0.3);
					  effectcolor1,Color.Alpha(Color.Orange,0.8)
		);
	}
};
--
local bShow = 0;
t[#t+1] = Def.ActorFrame {
	InitCommand=cmd(visible,false);
	ToggleConsoleDisplayMessageCommand=function(self)
		bShow = 1 - bShow;
		self:visible( bShow == 1 );
	end;
	-- Grid
--[[ 	LoadActor("_32") .. {
		InitCommand=cmd(x,SCREEN_CENTER_X;y,SCREEN_CENTER_Y;zoomto,SCREEN_WIDTH,SCREEN_HEIGHT;customtexturerect,0,0,SCREEN_WIDTH/32,SCREEN_HEIGHT/32);
		OnCommand=cmd(diffuse,color("0,0,0,0.5"));
	};
	LoadActor("_16") .. {
		InitCommand=cmd(x,SCREEN_CENTER_X;y,SCREEN_CENTER_Y;zoomto,SCREEN_WIDTH,SCREEN_HEIGHT;customtexturerect,0,0,SCREEN_WIDTH/16,SCREEN_HEIGHT/16);
		OnCommand=cmd(diffuse,color("1,1,1,0.125"));
	}; --]]
--[[ 	LoadActor("_8") .. {
		InitCommand=cmd(x,SCREEN_CENTER_X;y,SCREEN_CENTER_Y;zoomto,SCREEN_WIDTH,SCREEN_HEIGHT;customtexturerect,0,0,SCREEN_WIDTH/8,SCREEN_HEIGHT/8);
		OnCommand=cmd(diffuse,color("#00BFE833"));
	}; --]]
	-- Left
	Def.Quad {
		InitCommand=cmd(horizalign,left;x,SCREEN_LEFT;y,SCREEN_CENTER_Y;zoomto,16,SCREEN_HEIGHT);
		OnCommand=cmd(diffuse,color("0,0,0,0.5"));
	};
	-- Right
	Def.Quad {
		InitCommand=cmd(horizalign,right;x,SCREEN_RIGHT;y,SCREEN_CENTER_Y;zoomto,16,SCREEN_HEIGHT);
		OnCommand=cmd(diffuse,color("0,0,0,0.5"));
	};
};
--
return t
