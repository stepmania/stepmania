local function Beat(self)
	local this = self:GetChildren()
	--Yes, why not?
	this.Base:pause();
	this.Glow:pause();
	this.Tap:pause();
	this.Base:setstate(0);
	this.Glow:setstate(1);
	this.Tap:setstate(2);
	
	this.Glow:blend('BlendMode_Add');
	
	this.Glow:diffusealpha(0);
	local beat = GAMESTATE:GetSongBeat()
	local part = beat%1
	local eff = scale(part,0,0.5,1,0)
	if beat  >= 0 then
		this.Glow:diffusealpha(eff);
	end
end

return Def.ActorFrame {
	-- COMMANDS --
	InitCommand=cmd(SetUpdateFunction,Beat);
	-- LAYERS --
	NOTESKIN:LoadActor(Var "Button", "Ready Receptor")..{
		Name="Base";
		InitCommand=cmd();
	};
	NOTESKIN:LoadActor(Var "Button", "Ready Receptor")..{
		Name="Glow";
		InitCommand=cmd();
	};
	NOTESKIN:LoadActor(Var "Button", "Ready Receptor")..{
		Name="Tap";
		InitCommand=NOTESKIN:GetMetricA(Var "Button" ,"ReceptorTapInitCommand");
		PressCommand=NOTESKIN:GetMetricA(Var "Button" ,"ReceptorTapPressCommand");
	};
}