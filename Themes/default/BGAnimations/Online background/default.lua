-- You know what, I guess the "fancy UI background" theme option can be put to use.
if ThemePrefs.Get("FancyUIBG") then
	return Def.ActorFrame {
			Def.Quad {
				InitCommand=cmd(Center;zoomto,SCREEN_WIDTH,SCREEN_HEIGHT;diffuse,color("#18060C");diffusetopedge,color("#000000"));
			};
			
			LoadActor("_base") .. {
			InitCommand=cmd(Center;zoomto,SCREEN_WIDTH,SCREEN_HEIGHT;blend,'BlendMode_Add');
			},
			
			LoadActor("_barcode") .. {
				InitCommand=cmd(zoomto,36,1024;diffuse,color("#882D47");x,SCREEN_LEFT+15;y,SCREEN_CENTER_Y;diffusealpha,0.1);
				OnCommand=cmd(customtexturerect,0,0,1,1;texcoordvelocity,0,-0.1);
			};
			
			LoadActor("_barcode") .. {
				InitCommand=cmd(zoomto,36,1024;diffuse,color("#882D47");x,SCREEN_RIGHT-15;y,SCREEN_CENTER_Y;diffusealpha,0.1);
				OnCommand=cmd(customtexturerect,0,0,1,1;texcoordvelocity,0,0.1);
			};
		};
else
	return 	Def.ActorFrame {
		Def.Quad {
			InitCommand=cmd(Center;zoomto,SCREEN_WIDTH,SCREEN_HEIGHT;diffuse,color("#18060C");diffusetopedge,color("#000000"));
		};
			
		LoadActor("_base") .. {
			InitCommand=cmd(Center;zoomto,SCREEN_WIDTH,SCREEN_HEIGHT;blend,'BlendMode_Add');
		},
	};
end