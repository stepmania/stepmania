local Player = ...
assert(Player,"MachineRecord needs Player")
local stats = STATSMAN:GetCurStageStats():GetPlayerStageStats(Player);
local record = stats:GetMachineHighScoreIndex()
local hasMachineRecord = record ~= -1

return LoadFont("Common normal")..{
	Text=string.format(THEME:GetString("ScreenEvaluation", "MachineRecord"), record+1);
	InitCommand=cmd(zoom,0.55;shadowlength,1;NoStroke;glowshift;effectcolor1,color("1,1,1,0");effectcolor2,color("1,1,1,0.25"));
	BeginCommand=cmd(visible,hasMachineRecord;);
};