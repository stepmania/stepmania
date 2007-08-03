local Player = ...
assert( Player );

local t = Def.ActorFrame {
	Def.GraphDisplay {
		OnCommand=function(self)
			local Stats = STATSMAN:GetCurStageStats();
			self:LoadFromStats( Stats, Stats:GetPlayerStageStats(Player) )
		end,
		Texture = LoadActor("LifeGraph p1") .. {
			InitCommand=cmd(ztest,1;diffusealpha,0),
			OnCommand=cmd(linear,0.5;diffusealpha,1),
			OffCommand=cmd(linear,0.5;diffusealpha,0),
		},
		Line = Def.GraphLine { InitCommand=cmd(visible,false) },
		Body = Def.GraphBody { InitCommand=cmd(zwrite,1;blend,"BlendMode_NoEffect") },
		SongBoundary = Def.Actor { InitCommand=cmd(visible,false) },
		Barely = Def.Actor { InitCommand=cmd(visible,false) },
	}
}
return t
