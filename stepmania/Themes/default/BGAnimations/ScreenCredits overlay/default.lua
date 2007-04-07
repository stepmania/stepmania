local children = {
	LoadActor( "song background scroller" ) .. {
		InitCommand = cmd(x,SCREEN_CENTER_X-160;y,SCREEN_CENTER_Y);
	};
	LoadActor( "credits" ) .. {
		InitCommand = cmd(x,SCREEN_CENTER_X+160;y,SCREEN_CENTER_Y);
	};
--	Def.Quad {
--		InitCommand = cmd(diffuse,0,0,0,0;setsize,SCREEN_WIDTH,60;x,SCREEN_CENTER_X;y,SCREEN_TOP+30;diffusebottomedge,0,0,0,0);
--	};
--	Def.Quad {
--		InitCommand = cmd(diffuse,0,0,0,0;setsize,SCREEN_WIDTH,60;x,SCREEN_CENTER_X;y,SCREEN_BOTTOM-30;diffusetopedge,0,0,0,0);
--	}
}

return WrapInActorFrame( children )
