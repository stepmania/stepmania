local t = Def.ActorFrame {
	FOV=90;
	--
	Def.Quad {
		InitCommand=cmd(zoomto,SCREEN_CENTER_X+80,SCREEN_HEIGHT);
		OnCommand=cmd(diffuse,Color.Black;diffusealpha,0.75;fadeleft,32/SCREEN_CENTER_X;faderight,32/SCREEN_CENTER_X);
	};
	LoadActor( NOTESKIN:GetPathForNoteSkin("Center","Tap","cmd") ) .. {
		InitCommand=cmd(y,20);
	};
	LoadFont("Common Bold") .. {
		Text="PRESS";
		InitCommand=cmd(y,-20);
		OnCommand=cmd(shadowlength,1;pulse;effectmagnitude,1,1.125,1;effectperiod,0.5);
	};
	LoadFont("Common Bold") .. {
		Text="TO START";
		InitCommand=cmd(y,58);
		OnCommand=cmd(shadowlength,1;zoom,0.875);
	};
};

t.InitCommand=cmd(Center;x,SCREEN_CENTER_X*1.5;visible,false;diffusealpha,0);
t.StartSelectingStepsMessageCommand=cmd(linear,0.2;visible,true;diffusealpha,1);
t.StartSelectingSongMessageCommand=cmd(linear,0.2;visible,true;diffusealpha,0);
return t;
