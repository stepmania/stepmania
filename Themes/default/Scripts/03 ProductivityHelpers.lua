function HitCombo()
	sGame = GAMESTATE:GetCurrentGame():GetName();
	local Combo = {
		dance = 2,
		pump = 4,
		beat = 2,
		kb7 = 2,
		para = 2,
	};
	return Combo[sGame]
end;

function MissCombo()
	sGame = GAMESTATE:GetCurrentGame():GetName();
	local Combo = {
		dance = 2,
		pump = 4,
		beat = cmd(),
		kb7 = cmd(),
		para = cmd(),
	};
	return Combo[sGame]
end;

function FailCombo() -- The combo that causes game failure.
	sGame = GAMESTATE:GetCurrentGame():GetName();
	local Combo = {
		dance = "30", -- ITG/Pump Pro does it this way.
		pump = "51",
		beat = "-1",
		kb7 = "-1",
		para = "-1",
	};
	return Combo[sGame]
end;

function HoldJudgmentFail()
	if GAMESTATE:GetCurrentGame():GetName() == "pump" then
		return cmd();
	else return cmd(finishtweening;shadowlength,0;diffusealpha,1;zoom,1;y,-10;linear,0.8;y,10;sleep,0.5;linear,0.1;zoomy,0.5;zoomx,2;diffusealpha,0);
	end;
end;

function HoldJudgmentPass()
	if GAMESTATE:GetCurrentGame():GetName() == "pump" then
		return cmd();
	else return cmd(finishtweening;shadowlength,0;diffusealpha,1;zoom,1.25;linear,0.3;zoomx,1;zoomy,1;sleep,0.5;linear,0.1;zoomy,0.5;zoomx,2;diffusealpha,0);
	end;
end;