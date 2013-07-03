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
			InitCommand=function(self)
				self:y(_ * 40);
				self:diffuse(color("#000000"));
				self:Stroke(color("#ffffff"));
			end;
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
			InitCommand=function(self)
				self:y(_ * 40);
				self:diffuse(color("#000000"));
				self:Stroke(color("#ffffff"));
			end;
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
	}
end

return Def.ActorFrame {
	LoadFont("_impact 50px")..{
		Text="Evaluation";
		InitCommand=function(self)
			self:CenterX();
			self:y(50);
			self:diffuse(color("#000000"));
			self:Stroke(color("#ffffff"));
		end;
	};
	Labels()..{
		InitCommand=function(self)
			self:CenterX();
			self:FromTop(130);
		end;
	};
	Scores(PLAYER_1)..{
		InitCommand=function(self)
			self:FromCenterX(-160);
			self:FromTop(130);
		end;
	};
	Scores(PLAYER_2)..{
		InitCommand=function(self)
			self:FromCenterX(160);
			self:FromTop(130);
		end;
	};
	Grade(PLAYER_1)..{
		InitCommand=function(self)
			self:FromLeft(50);
			self:FromTop(240);
		end;
	};
	Grade(PLAYER_2)..{
		InitCommand=function(self)
			self:FromRight(-50);
			self:FromTop(240);
		end;
	};
}