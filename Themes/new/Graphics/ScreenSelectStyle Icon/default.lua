local gc = Var("GameCommand");
--
local t = Def.ActorFrame {};
--
t[#t+1] = Def.ActorFrame { 
	Def.Quad {
		InitCommand=cmd(zoomto,96,24;diffuse,Color.White);
		GainFocusCommand=cmd(diffuseshift;effectcolor2,Color.White;effectcolor1,Color.Blue);
		LoseFocusCommand=cmd(stopeffect;);
		DisabledCommand=cmd(stopeffect;diffuse,color("0.5,0.5,0.5,1"));
		EnabledCommand=cmd(stopeffect;diffuse,Color.White);
	};
	--
	LoadFont("Common Normal") .. {
		Text=gc:GetText();
		InitCommand=cmd(diffuse,Color.Black;zoom,0.675);
	};
};
return t