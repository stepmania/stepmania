local Player = ...
assert(Player,"PersonalRecord needs Player")
local stats = STATSMAN:GetCurStageStats():GetPlayerStageStats(Player);
local record = stats:GetPersonalHighScoreIndex()
local hasPersonalRecord = record ~= -1

return LoadFont("Common normal")..{
	Text=string.format(THEME:GetString("ScreenEvaluation", "PersonalRecord"), record+1)
	InitCommand=cmd(zoom,0.55;shadowlength,1;NoStroke;glowshift;effectcolor1,color("1,1,1,0");effectcolor2,color("1,1,1,0.25"));
	BeginCommand=cmd(visible,hasPersonalRecord;);
};