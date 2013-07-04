local Player = ...
assert(Player,"PersonalRecord needs Player")
local stats = STATSMAN:GetCurStageStats():GetPlayerStageStats(Player);
local record = stats:GetPersonalHighScoreIndex()
local hasPersonalRecord = record ~= -1

return LoadFont("Common normal")..{
	InitCommand=function(self)
		self:zoom(0.55);
		self:shadowlength(1);
		self:NoStroke();
		self:glowshift();
		self:effectcolor1(color("1,1,1,0"));
		self:effectcolor2(color("1,1,1,0.25"));
	end;
	BeginCommand=function(self)
		self:visible(hasPersonalRecord);
		local text = string.format(THEME:GetString("ScreenEvaluation", "PersonalRecord"), record+1)
		self:settext(text)
	end
};