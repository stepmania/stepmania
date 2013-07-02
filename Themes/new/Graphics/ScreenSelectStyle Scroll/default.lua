local gc = Var("GameCommand");
--
local t = Def.ActorFrame {};
--
t[#t+1] = Def.ActorFrame { 
	Def.Quad {
		InitCommand=function(self)
			self:zoomto(96, 24);
			self:diffuse(Color.White);
		end;
	};
};
return t