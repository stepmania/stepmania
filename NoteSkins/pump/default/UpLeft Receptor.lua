local function Beat(self)
	local this = self:GetChildren()
	
	local beat = GAMESTATE:GetSongBeat()
	
	local part = beat%1
	part = clamp(part,0,0.5)
	local eff = scale(part,0,0.5,1,0)
	if (GAMESTATE:GetSongDelay() or false) and part == 0 then eff = 0 end
	if beat < 0 then
		eff = 0
	end
	this.Glow:diffusealpha(eff);
end

return Def.ActorFrame {
	-- COMMANDS --
	InitCommand=cmd(SetUpdateFunction,Beat);
	
	-- LAYERS --
	NOTESKIN:LoadActor("Center", "Outline Receptor")..{
		Name="Outline";
		Condition=Var "Button" == "Center";
		--InitCommand=cmd(x,96);
	};
	NOTESKIN:LoadActor(Var "Button", "Ready Receptor")..{
		Name="Base";
		Frames = { { Frame = 0 } };
		--PressCommand=cmd(finishtweening;glow,1,1,1,1;linear,0.1;glow,1,1,1,0);
	};
	NOTESKIN:LoadActor(Var "Button", "Ready Receptor")..{
		Name="Glow";
		Frames = { { Frame = 1 } };
		InitCommand=cmd(blend,'BlendMode_Add');
		--PressCommand=cmd(finishtweening;linear,0.05;zoom,0.9;linear,0.1;zoom,1);
	};
	--[[
	NOTESKIN:LoadActor(Var "Button", "Ready Receptor")..{
		Name="Tap";
		Frames = { { Frame = 2 } };
		InitCommand=cmd(zoom,1;diffusealpha,0;glow,1,1,1,0);
		--NOTESKIN:GetMetricA(Var "Button", "TapInitCommand");
		--
		PressCommand=cmd(finishtweening;glow,1,1,1,1;zoom,1;linear,0.2;glow,1,1,1,0;zoom,1.2);
		--NOTESKIN:GetMetricA(Var "Button", "TapHeldCommand");
		--
	};
	--]]
}