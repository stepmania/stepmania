local t = Def.ActorFrame{};

t[#t+1] = LoadFont("Common normal")..{
	InitCommand=cmd(x,SCREEN_CENTER_X*0.5;y,SCREEN_CENTER_Y);
	SetCommand=function(self)
		local song = GAMESTATE:GetCurrentSong();
		if song then
			--[[
			-- test of TimingData:GetBPMs
			local timing = song:GetTimingData();
			local bpms = timing:GetBPMs();
			local text = "";
			for i=1,#bpms do
				text = text ..bpms[i];
				if i < (#bpms) then
					text = text..", ";
				end;
			end;
			self:settext(text);
			--]]
			--[[
			-- test of TimingData:GetStops
			-- format:
			-- start row (float as beat)=stop length (float as seconds)
			local timing = song:GetTimingData();
			local stops = timing:GetStops();
			local text = "";
			for i=1,#stops do
				text = text ..stops[i];
				if i < (#stops) then
					text = text..", ";
				end;
			end;
			self:settext(text);
			--]]
			--[[
			-- test of TimingData:GetBPMsAndTimes
			-- format:
			-- start row (float as beat)=new bpm (float)
			local timing = song:GetTimingData();
			local bpmchanges = timing:GetBPMsAndTimes();
			local text = "";
			for i=1,#bpmchanges do
				text = text ..bpmchanges[i];
				if i < (#bpmchanges) then
					text = text..",\n";
				end;
			end;
			self:settext(text);
			--]]
			-- test of TimingData:GetActualBPM (may need to be renamed)
			-- bpms[1] = min, bpms[2] = max
			local timing = song:GetTimingData();
			local bpms = timing:GetActualBPM();
			local minBPM = bpms[1];
			local maxBPM = bpms[2];
			local text = "";
			if minBPM == maxBPM then
				text = "min = max = ".. maxBPM;
			else
				text = "min = ".. minBPM .." | max = ".. maxBPM;
			end;
			self:settext(text);
		else
			self:settext("");
		end;
	end;
	CurrentSongChangedMessageCommand=cmd(playcommand,"Set");
	CurrentCourseChangedMessageCommand=cmd(playcommand,"Set");
	CurrentStepsP1ChangedMessageCommand=cmd(playcommand,"Set");
	CurrentTrailP1ChangedMessageCommand=cmd(playcommand,"Set");
	CurrentStepsP2ChangedMessageCommand=cmd(playcommand,"Set");
	CurrentTrailP2ChangedMessageCommand=cmd(playcommand,"Set");
};

return t;