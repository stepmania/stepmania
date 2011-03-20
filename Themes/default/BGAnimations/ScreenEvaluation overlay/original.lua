local ss = STATSMAN:GetCurStageStats();
local pss = ss:GetPlayerStageStats('PlayerNumber_P1');
local misscount = pss:GetTapNoteScores('TapNoteScore_Miss');
local boocount = pss:GetTapNoteScores('TapNoteScore_W5');
local goodcount = pss:GetTapNoteScores('TapNoteScore_W4');
local greatcount = pss:GetTapNoteScores('TapNoteScore_W3');
local perfcount = pss:GetTapNoteScores('TapNoteScore_W2');
local marvcount = pss:GetTapNoteScores('TapNoteScore_W1');
local minehitcount = pss:GetTapNoteScores('TapNoteScore_HitMine');
local minemisscount = pss:GetTapNoteScores('TapNoteScore_AvoidMine');
local okcount = pss:GetHoldNoteScores('HoldNoteScore_Held');
local ngcount = pss:GetHoldNoteScores('HoldNoteScore_LetGo');
local ITGdp = marvcount*7 + perfcount*6 + greatcount*5 + goodcount*4 + boocount*2 + okcount*7
local ITGdpmax = (marvcount + perfcount + greatcount + goodcount + boocount + misscount + okcount + ngcount)*7
local MIGSdp = marvcount*3 + perfcount*2 + greatcount*1 - boocount*4 - misscount*8 + okcount*6
local MIGSdpmax = (marvcount + perfcount + greatcount + goodcount + boocount + misscount)*3 + (okcount + ngcount)*6
local histogram = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
local total = 0
local maxValue = 0;
print("John11length: "..#histogram);

for i=1,#tTotalJudgmentsSigned do
	print("John11Timings: "..i.." - "..tTotalJudgmentsSigned[i])
	local index = math.floor((0.18-tTotalJudgmentsSigned[i])*(#histogram/.36));
	print("John11Index: "..i.." - "..index)
	if index >= 0 and index <= #histogram - 1 then
		histogram[index] = histogram[index] + 1
		total = total + 1
		if histogram[index] > maxValue then 
			maxValue = histogram[index]
		end
	end
end

for i=1,#histogram do
	print("John11Judgments: "..i.." - "..histogram[i])
end

local t = Def.ActorFrame {}
t[#t+1] = Def.ActorFrame {
	InitCommand=cmd(x,SCREEN_CENTER_X/3;y,SCREEN_CENTER_Y);
	LoadFont("Common Normal") .. {
		Text="MIGS DP: "..tostring(MIGSdp).."/"..tostring(MIGSdpmax).."\nITG DP: "..tostring(ITGdp).."/"..tostring(ITGdpmax).."\nOffset Avg: "..RoundTo(tTimingDifferenceAverage,5).."\nAbs Offset Avg: "..RoundTo(tTimingDifferenceAbsAverage,5).."\nEarly Taps: "..tEarlyHits.."\nLate Taps: "..tLateHits;
		InitCommand=cmd(y,-4;shadowlength,1;diffuse,Color("Red");zoom,0.5)
	};
}

for i=1,#histogram do
	local offset = -1
	if i > #histogram/2 then
		offset = 1
	end
	t[#t+1] = Def.Quad {
		InitCommand=cmd(diffuse,Color("Red");zoomtowidth,300/#histogram-1;zoomtoheight,histogram[#histogram-i]/maxValue*150;x,SCREEN_CENTER_X+100+300/#histogram*i+offset;y,SCREEN_CENTER_Y-histogram[#histogram-i]/maxValue*150/2);
	}
end

t[#t+1] = Def.Quad {
	InitCommand=cmd(x,SCREEN_CENTER_X+250;y,SCREEN_CENTER_Y+3;zoomtowidth,300;zoomtoheight,5;diffuse,color("#FFFFFF"));
}

t[#t+1] = Def.Quad {
	InitCommand=cmd(x,SCREEN_CENTER_X+252;y,SCREEN_CENTER_Y-75;zoomtowidth,1;zoomtoheight,150;diffuse,color("#000000"));
}


t[#t+1] = Def.ActorFrame {
	InitCommand=cmd(x,SCREEN_CENTER_X+125;y,SCREEN_CENTER_Y+15);
	LoadFont("Common Normal") .. {
		Text="Early";
		InitCommand=cmd(shadowlength,1;diffuse,Color("#FF0000");zoom,0.5)
	};
}

t[#t+1] = Def.ActorFrame {
	InitCommand=cmd(x,SCREEN_CENTER_X+375;y,SCREEN_CENTER_Y+15);
	LoadFont("Common Normal") .. {
		Text="Late";
		InitCommand=cmd(shadowlength,1;diffuse,Color("#FF0000");zoom,0.5)
	};
}

print("John11Total: "..total)
print("John11MaxValue: "..maxValue)

return t