local t = Def.ActorFrame {
	Def.ActorFrame {
		LoadActor("star field") .. {
			InitCommand=cmd(x,SCREEN_CENTER_X;y,SCREEN_CENTER_Y;zoom,16;customtexturerect,0,0,16,16;texcoordvelocity,1,0;);
		};
		LoadActor("black circle") .. {
			InitCommand=cmd(x,SCREEN_CENTER_X;y,SCREEN_CENTER_Y);
		};
		LoadActor("game star") .. {
			InitCommand=cmd(x,SCREEN_CENTER_X-130;y,SCREEN_CENTER_Y-50;);
			OnCommand=cmd(addx,SCREEN_WIDTH;sleep,0.0;decelerate,0.5;addx,-SCREEN_WIDTH;);
		};
		LoadActor("over star") .. {
			InitCommand=cmd(x,SCREEN_CENTER_X;y,SCREEN_CENTER_Y+20;);
			OnCommand=cmd(addx,SCREEN_WIDTH;sleep,0.2;decelerate,0.5;addx,-SCREEN_WIDTH;);
		};
		LoadActor("game text") .. {
			InitCommand=cmd(x,SCREEN_CENTER_X-60;y,SCREEN_CENTER_Y-50+4;);
			OnCommand=cmd(addx,SCREEN_WIDTH;sleep,0.5;decelerate,0.5;addx,-SCREEN_WIDTH;);
		};
		LoadActor("over text") .. {
			InitCommand=cmd(x,SCREEN_CENTER_X+66;y,SCREEN_CENTER_Y+20+4;);
			OnCommand=cmd(addx,SCREEN_WIDTH;sleep,0.7;decelerate,0.5;addx,-SCREEN_WIDTH;);
		};
		Def.ActorFrame {
			InitCommand=cmd(x,SCREEN_CENTER_X+114;y,SCREEN_CENTER_Y-50;);
			OnCommand=cmd(addx,SCREEN_WIDTH;sleep,1;rotationz,360*2;decelerate,1;addx,-SCREEN_WIDTH;rotationz,0;);

			LoadActor("face open") .. {
			};
			LoadActor("face closed") .. {
				OnCommand=cmd(diffusealpha,0;sleep,3;diffusealpha,1;sleep,0.5;diffusealpha,0;);
			};
			Def.ActorFrame {
				InitCommand=cmd(x,-54;y,-60;zoom,0;sleep,2;bounceend,0.3;zoom,1;);
				LoadActor("balloon") .. {
				};
				LoadFont("_venacti Bold 15px") .. {
					InitCommand=cmd(y,-5;settext,"See you next time!";diffuse,color("#3f4c72");shadowcolor,color("#dcb2f2aa");wrapwidthpixels,100;skewx,-0.1;shadowlength,2;);
				};
			};
		};

	};
};

return t;
