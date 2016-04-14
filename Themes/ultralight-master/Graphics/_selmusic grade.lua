local player = ...
assert(player, "needs a player")

return Def.ActorFrame{
	LoadFont("Common normal")..{
		InitCommand=cmd(zoom,0.75;diffuse,color("#000000");shadowlength,0;strokecolor,color("#00000000"));
		SetGradeCommand=function(self)
			
		end;
	};
};