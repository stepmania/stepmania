local function Beat(self)
	local this = self:GetChildren()
	this.Glowpart:diffusealpha(0);
	local beat = GAMESTATE:GetSongBeat()
	local part = beat%1
	local eff = scale(part,0,0.5,1,0)
	if beat >=0 then
		this.Glowpart:diffusealpha(eff);
	end
end

return Def.ActorFrame {
	InitCommand=cmd(SetUpdateFunction,Beat);
	LoadActor("_receptor parts")..{
		InitCommand=cmd(pause;setstate,1);
	};
	LoadActor("_receptor parts")..{
		Name="Glowpart";
		InitCommand=function(self)
			--(NOTESKIN:GetMetricA("Receptor","BeatCommand"))(self);
			self:pause();
			self:setstate(4);
			self:blend('BlendMode_Add');
		end;
	};
	LoadActor("_receptor parts")..{
		InitCommand=cmd(pause;setstate,7;zoom,1;diffusealpha,0;glow,1,1,1,0);
		PressCommand=cmd(finishtweening;glow,1,1,1,1;zoom,1;linear,0.2;glow,1,1,1,0;zoom,1.2);
	};
}