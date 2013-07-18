local function Labels()
	local captions = {
		"Perfect",
		"Great",
		"Good",
		"Bad",
		"Miss",
		"Combo",
		"Score",
	};
	
	local t = Def.ActorFrame {}
	
	for idx,k in ipairs(captions) do
		local _ = idx-1
		t[#t+1] = LoadFont("_arial black 20px")..{
			Text=k;
			InitCommand=cmd(y,_*40;diffuse,color("#000000");Stroke,color("#ffffff"));
		};
	end
	
	return t
end

local function Scores(player)
	if not player or not GAMESTATE:IsPlayerEnabled(player) then return Def.Actor {} end
	
	local curstats = STATSMAN:GetCurStageStats():GetPlayerStageStats(player)
	
	local timmings = {
		{
			curstats:GetTapNoteScores('TapNoteScore_CheckpointHit'),
			curstats:GetTapNoteScores('TapNoteScore_W1'),
			curstats:GetTapNoteScores('TapNoteScore_W2'),
		},
		{
			curstats:GetTapNoteScores('TapNoteScore_W3')
		},
		{
			curstats:GetTapNoteScores('TapNoteScore_W4')
		},
		{
			curstats:GetTapNoteScores('TapNoteScore_W5')
		},
		{
			curstats:GetTapNoteScores('TapNoteScore_CheckpointMiss'),
			curstats:GetTapNoteScores('TapNoteScore_Miss'),
		},
		{
			curstats:MaxCombo()
		},
		{
			curstats:GetScore()
		}
	}
	
	local t = Def.ActorFrame {}
	
	for idx,k in ipairs(timmings) do
		local _ = idx-1
		local curstats = STATSMAN:GetCurStageStats():GetPlayerStageStats(player)
		local number = 0
		
		for i,v in ipairs(k) do
			number = number + v-- curstats:GetTapNoteScores(v);
		end
		
		t[#t+1] = LoadFont("_arial black 20px")..{
			Text=string.format("%03d", number); --curstats:GetTapNoteScores(k)
			InitCommand=cmd(y,_*40;diffuse,color("#000000");Stroke,color("#ffffff"));
		};
	end
	
	return t
end

local function Grade(player)
	if not player or not GAMESTATE:IsPlayerEnabled(player) then return Def.Actor {} end
	
	local captions = {
		Grade_Tier01 = "S";
		Grade_Tier02 = "S";
		Grade_Tier03 = "A";
		Grade_Tier04 = "B";
		Grade_Tier05 = "C";
		Grade_Tier06 = "D";
		Grade_Tier07 = "F";
		Grade_Failed = "S";
	}
	local grade = STATSMAN:GetCurStageStats():GetPlayerStageStats(player):GetGrade()
	
	return LoadFont("_impact 50px")..{
		Text=captions[grade];
		--OnCommand=cmd(zoom,2;diffuse,color("#000000");Stroke,color("#ffffff");diffusealpha,0;sleep,1;linear,1;zoom,1;diffuse,color("#000000");Stroke,color("#ffffff");;diffusealpha,1);
	}
end

return Def.ActorFrame {
	LoadFont("_impact 50px")..{
		Text="Evaluation";
		InitCommand=cmd(CenterX;y,50;diffuse,color("#000000");Stroke,color("#ffffff"));
	};
	Labels()..{
		InitCommand=cmd(CenterX;FromTop,130);
	};
	Scores(PLAYER_1)..{
		InitCommand=cmd(FromCenterX,-160;FromTop,130);
	};
	Scores(PLAYER_2)..{
		InitCommand=cmd(FromCenterX,160;FromTop,130);
	};
	Grade(PLAYER_1)..{
		InitCommand=cmd(FromLeft,50;FromTop,240);
	};
	Grade(PLAYER_2)..{
		InitCommand=cmd(FromRight,-50;FromTop,240);
	};
}