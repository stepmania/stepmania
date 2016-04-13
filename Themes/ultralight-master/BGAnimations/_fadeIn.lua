local time = ...
if not time then time = 0.5; end

return Def.ActorFrame{
	Def.Quad{
		InitCommand=cmd(FullScreen;diffuse,color("#000000FF"));
		OnCommand=cmd(linear,time;diffusealpha,0);
	};
};