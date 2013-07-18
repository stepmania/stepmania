local gc = Var("GameCommand");
--
local t = Def.ActorFrame {};
--
t[#t+1] = Def.ActorFrame { 
	Def.Quad {
		InitCommand=cmd(zoomto,96,24;diffuse,Color.White);
	};
};
return t