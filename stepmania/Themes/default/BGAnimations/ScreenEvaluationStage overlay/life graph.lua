local Player = ...
assert( Player );

local t = Def.ActorFrame {
children = {
	Def.GraphDisplay {
		OnCommand=function(self)
			local Stats = STATSMAN:GetCurStageStats();
			self:LoadFromStats( Stats, Stats:GetPlayerStageStats(Player) )
		end,
		-- Input = { Name="Player" },
		Texture = Def.Layer {
			File="LifeGraph p1",
			InitCommand=cmd(ztest,1;diffusealpha,0),
			OnCommand=cmd(linear,0.5;diffusealpha,1),
			OffCommand=cmd(linear,0.5;diffusealpha,0),
		},
		Line = Def.Layer { InitCommand=cmd(hidden,1) },
		Body = Def.Layer { InitCommand=cmd(zwrite,1;blend,"BlendMode_NoEffect") },
		SongBoundary = Def.Actor { InitCommand=cmd(hidden,1) },
		Barely = Def.Actor { InitCommand=cmd(hidden,1) },
	}
}
}
return t
