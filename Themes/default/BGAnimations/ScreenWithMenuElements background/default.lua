if ThemePrefs.Get("FancyUIBG") then
	return Def.ActorFrame {
		
		LoadActor(THEME:GetPathG("common bg", "base")) .. {
			InitCommand=cmd(Center;zoomto,SCREEN_WIDTH,SCREEN_HEIGHT)
		},
		
		LoadActor("_maze") .. {
			OnCommand=cmd(Center;diffuse,color("#f6784922");effectperiod,10;spin;effectmagnitude,0,0,2.2)
		},
		
		LoadActor("_barcode") .. {
			InitCommand=cmd(zoomto,36,1024;blend,'BlendMode_Add';x,SCREEN_LEFT+6;y,SCREEN_CENTER_Y;diffusealpha,0.08);
			OnCommand=cmd(customtexturerect,0,0,1,1;texcoordvelocity,0,-0.1);
		};
		LoadActor("_barcode") .. {
			InitCommand=cmd(zoomto,36,1024;blend,'BlendMode_Add';x,SCREEN_RIGHT-6;y,SCREEN_CENTER_Y;diffusealpha,0.08);
			OnCommand=cmd(customtexturerect,0,0,1,1;texcoordvelocity,0,0.1);
		};
		
		Def.ActorFrame {
		OnCommand=cmd(diffusealpha,0;decelerate,1.8;diffusealpha,1);
			LoadActor("_tunnel1") .. {
				InitCommand=cmd(x,SCREEN_LEFT+160;y,SCREEN_CENTER_Y;blend,'BlendMode_Add';rotationz,-20),
				OnCommand=cmd(zoom,1.75;diffusealpha,0.14;spin;effectmagnitude,0,0,16.5)
			};	
			LoadActor("_tunnel1") .. {
				InitCommand=cmd(x,SCREEN_LEFT+160;y,SCREEN_CENTER_Y;blend,'BlendMode_Add';rotationz,-10),
				OnCommand=cmd(zoom,1.0;diffusealpha,0.12;spin;effectmagnitude,0,0,-11)
			};
			LoadActor("_tunnel1") .. {
				InitCommand=cmd(x,SCREEN_LEFT+160;y,SCREEN_CENTER_Y;blend,'BlendMode_Add';rotationz,0),
				OnCommand=cmd(zoom,0.5;diffusealpha,0.10;spin;effectmagnitude,0,0,5.5)
			};		
			LoadActor("_tunnel1") .. {
				InitCommand=cmd(x,SCREEN_LEFT+160;y,SCREEN_CENTER_Y;blend,'BlendMode_Add';rotationz,-10),
				OnCommand=cmd(zoom,0.2;diffusealpha,0.08;spin;effectmagnitude,0,0,-2.2)
			};
	};
	}
else
	return 	LoadActor(THEME:GetPathG("common bg", "base")) .. {
		InitCommand=cmd(Center;zoomto,SCREEN_WIDTH,SCREEN_HEIGHT)
	}
end