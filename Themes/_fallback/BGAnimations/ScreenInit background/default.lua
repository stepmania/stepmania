local t = Def.ActorFrame {};

t[#t+1] = Def.ActorFrame {
  InitCommand=cmd(Center);
	Def.Quad {
		InitCommand=cmd(scaletoclipped,SCREEN_WIDTH,SCREEN_HEIGHT);
		OnCommand=cmd(diffuse,color("0,0,0,1");diffusetopedge,color("0.1,0.1,0.1,1"));
	};
};
t[#t+1] = Def.ActorFrame {
  InitCommand=cmd(Center);
	Def.ActorFrame {
		Def.Quad {
			InitCommand=cmd(zoomto,SCREEN_WIDTH,128);
			OnCommand=cmd(diffusealpha,1;sleep,1.5;linear,0.25);
		};
		LoadActor("ssc") .. {
			OnCommand=cmd(diffusealpha,0;linear,1;diffusealpha,1;sleep,0.75;linear,0.25;diffusealpha,0);
		};
	};
	Def.ActorFrame {
	  OnCommand=cmd(playcommandonchildren,"ChildrenOn");
	  ChildrenOnCommand=cmd(diffusealpha,0;sleep,2;linear,0.25;diffusealpha,1);
		LoadFont("Common Normal") .. {
			Text=ProductID();
			InitCommand=cmd(y,-20;zoom,0.75);
			OnCommand=cmd(diffuse,color("0,0,0,1"));
		};
		LoadFont("Common Normal") .. {
			Text=THEME:GetThemeDisplayName();
			OnCommand=cmd(diffuse,color("0,0,0,1"));
		};
		LoadFont("Common Normal") .. {
			Text="Created by " .. THEME:GetThemeAuthor();
			InitCommand=cmd(y,24;zoom,0.75);
			OnCommand=cmd(diffuse,color("0,0,0,1"));
		};
	};
};

return t
