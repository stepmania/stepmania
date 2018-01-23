return Def.ActorFrame {
			Def.Quad {
				InitCommand=cmd(Center;zoomto,SCREEN_WIDTH,SCREEN_HEIGHT;diffuse,color("#093A3E");diffusetopedge,color("#000000"););
			};
			
			LoadActor("_base") .. {
			InitCommand=cmd(Center;zoomto,SCREEN_WIDTH,SCREEN_HEIGHT;blend,'BlendMode_Add';);
			},
			
			LoadActor("_barcode") .. {
				InitCommand=cmd(zoomto,36,1024;diffuse,color("#65F0FD");x,SCREEN_LEFT+15;y,SCREEN_CENTER_Y;diffusealpha,0.1);
				OnCommand=cmd(customtexturerect,0,0,1,1;texcoordvelocity,0,-0.1);
			};
			
			LoadActor("_barcode") .. {
				InitCommand=cmd(zoomto,36,1024;diffuse,color("#65F0FD");x,SCREEN_RIGHT-15;y,SCREEN_CENTER_Y;diffusealpha,0.1);
				OnCommand=cmd(customtexturerect,0,0,1,1;texcoordvelocity,0,0.1);
			};
	};