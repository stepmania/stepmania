local gc = Var "GameCommand";
local c = {};
	c.X = THEME:GetMetric( Var "LoadingScreen", "Icon" .. gc:GetName() .. "X");
	c.Y = THEME:GetMetric( Var "LoadingScreen", "Icon" .. gc:GetName() .. "Y");
local t = Def.ActorFrame {};
t[#t+1] = Def.ActorFrame {
	Condition=( gc:GetName() ~= "Back" );
	InitCommand=cmd(x,c.X;y,c.Y);
	GainFocusCommand=cmd(finishtweening;zoom,1.125;bounceend,0.125;zoom,1);
	LoseFocusCommand=cmd(stoptweening;linear,0.125;zoom,0.875);
	LoadActor("_base") .. {
		GainFocusCommand=cmd(stoptweening;linear,0.125;diffuse,Color("Orange");diffusetopedge,Color("Yellow"));
		LoseFocusCommand=cmd(stoptweening;linear,0.125;diffuse,Color("White"));
	};
	LoadFont("Common Normal") .. {
		Text=gc:GetName();
		InitCommand=cmd(strokecolor,Color("White"));
		OnCommand=cmd(diffuse,Color("Black"));
	};
};
t[#t+1] = Def.ActorFrame {
	Condition=( gc:GetName() == "Back" );
	InitCommand=cmd(x,c.X;y,c.Y);
	GainFocusCommand=cmd(finishtweening;zoom,1.125;bounceend,0.125;zoom,1);
	LoseFocusCommand=cmd(stoptweening;linear,0.125;zoom,0.875);
	LoadActor("_base") .. {
		GainFocusCommand=cmd(stoptweening;linear,0.125;diffuse,Color("Red"));
		LoseFocusCommand=cmd(stoptweening;linear,0.125;diffuse,Color("White"));
	};
	LoadFont("Common Normal") .. {
		Text=gc:GetName();
		InitCommand=cmd(strokecolor,Color("White"));
		OnCommand=cmd(diffuse,Color("Black"));
	};
};

return t;