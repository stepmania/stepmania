local vStats = STATSMAN:GetCurStageStats();

local function CreateStats( pnPlayer )
	-- Actor Templates
	local aLabel = LoadFont("Common Normal") .. { 
		InitCommand=function(self)
			self:zoom(0.5);
			self:shadowlength(1);
			self:horizalign(left);
		end;
	};
	local aText = LoadFont("Common Normal") .. { 
		InitCommand=function(self)
			self:zoom(0.5);
			self:shadowlength(1);
			self:horizalign(left);
		end;
	};
	-- DA STATS, JIM!!
	local pnStageStats = vStats:GetPlayerStageStats( pnPlayer );
	-- Organized Stats.
	local tStats = {
		W1			= pnStageStats:GetTapNoteScores('TapNoteScore_W1');
		W2			= pnStageStats:GetTapNoteScores('TapNoteScore_W2');
		W3			= pnStageStats:GetTapNoteScores('TapNoteScore_W3');
		W4			= pnStageStats:GetTapNoteScores('TapNoteScore_W4');
		W5			= pnStageStats:GetTapNoteScores('TapNoteScore_W5');
		Miss		= pnStageStats:GetTapNoteScores('TapNoteScore_Miss');
		HitMine		= pnStageStats:GetTapNoteScores('TapNoteScore_HitMine');
		AvoidMine	= pnStageStats:GetTapNoteScores('TapNoteScore_AvoidMine');
		Held		= pnStageStats:GetHoldNoteScores('HoldNoteScore_Held');
		LetGo		= pnStageStats:GetHoldNoteScores('HoldNoteScore_LetGo');
	};
  local itg_values= {W1= 5,
                     W2= 4,
                     W3= 2,
                     W4= 0,
                     W5= -6,
                     Miss= -12,
                     HitMine= 0, -- Kyz hates mines.  Should be -6
                     AvoidMine= 0,
                     Held= 5,
                     LetGo= 0}
  local migs_values= {W1= 3,
                     W2= 2,
                     W3= 1,
                     W4= 0,
                     W5= -4,
                     Miss= -8,
                     HitMine= 0,
                     AvoidMine= 0,
                     Held= 6,
                     LetGo= 0}
  local itg_keys_for_max= {W1= 5, W2= 5, W3= 5, W4= 5, W5= 5, Miss= 5, Held= 5, LetGo= 5}
  local migs_keys_for_max= {W1= 3, W2= 3, W3= 3, W4= 3, W3= 3, Miss= 3, Held= 6, LetGo= 6}
  
  local tValues= { ITG= 0, ITG_MAX= 0, MIGS= 0, MIGS_MAX= 0}
  for k, v in pairs(itg_keys_for_max) do
     if tStats[k] then
        tValues.ITG_MAX= tValues.ITG_MAX + (tStats[k] * v)
     end
  end
  for k, v in pairs(migs_keys_for_max) do
     if tStats[k] then
        tValues.MIGS_MAX= tValues.MIGS_MAX + (tStats[k] * v)
     end
  end
  
  for k, v in pairs(tStats) do
     if itg_values[k] then
        tValues.ITG= tValues.ITG + (v * itg_values[k])
     end
     if migs_values[k] then
        tValues.MIGS= tValues.MIGS + (v * migs_values[k])
     end
  end

	local t = Def.ActorFrame {};
	t[#t+1] = Def.ActorFrame {
		InitCommand=function(self)
			self:y(-34);
		end;
		LoadActor(THEME:GetPathG("ScreenTitleMenu","PreferenceFrame")) .. {
			InitCommand=function(self)
				self:zoom(0.875);
				self:diffuse(PlayerColor( pnPlayer ));
			end;
		};
		aLabel .. { 
			Text=THEME:GetString("ScreenEvaluation","ITG DP:");
			InitCommand=function(self)
				self:x(-64);
			end;
		};
		aText .. { 
			Text=string.format("%04i",tValues["ITG"]); 
			InitCommand=function(self)
				self:x(-8);
				self:y(5);
				self:vertalign(bottom);
				self:zoom(0.6);
			end;
		};
		aText .. { 
			Text="/"; 
			InitCommand=function(self)
				self:x(28);
				self:y(5);
				self:vertalign(bottom);
				self:zoom(0.5);
				self:diffusealpha(0.5);
			end;
		};
		aText .. { 
			Text=string.format("%04i",tValues["ITG_MAX"]); 
			InitCommand=function(self)
				self:x(32);
				self:y(5);
				self:vertalign(bottom);
				self:zoom(0.5);
			end;
		};
	};
	t[#t+1] = Def.ActorFrame {
		InitCommand=function(self)
			self:y(-6);
		end;
		LoadActor(THEME:GetPathG("ScreenTitleMenu","PreferenceFrame")) .. {
			InitCommand=function(self)
				self:zoom(0.875);
				self:diffuse(PlayerColor( pnPlayer ));
			end;
		};
		aLabel .. { 
			Text=THEME:GetString("ScreenEvaluation","MIGS DP:"); 
			InitCommand=function(self)
				self:x(-64);
			end;
		};
		aText .. { 
			Text=string.format("%04i",tValues["MIGS"]); 
			InitCommand=function(self)
				self:x(-8);
				self:y(5);
				self:vertalign(bottom);
				self:zoom(0.6);
			end;
		};
		aText .. { 
			Text="/"; 
			InitCommand=function(self)
				self:x(28);
				self:y(5);
				self:vertalign(bottom);
				self:zoom(0.5);
				self:diffusealpha(0.5);
			end;
		};
		aText .. { 
			Text=string.format("%04i",tValues["MIGS_MAX"]); 
			InitCommand=function(self)
				self:x(32);
				self:y(5);
				self:vertalign(bottom);
				self:zoom(0.5);
			end;
		};
	};
	return t
end;

local t = Def.ActorFrame {};
GAMESTATE:IsPlayerEnabled(PLAYER_1)
t[#t+1] = Def.ActorFrame {
	InitCommand=function(self)
		self:hide_if(not GAMESTATE:IsPlayerEnabled(PLAYER_1));
		self:x(WideScale(math.floor(SCREEN_CENTER_X * 0.3) - 8, math.floor(SCREEN_CENTER_X * 0.5) - 8));
		self:y(SCREEN_CENTER_Y-34);
	end;
	CreateStats( PLAYER_1 );
};
t[#t+1] = Def.ActorFrame {
	InitCommand=function(self)
		self:hide_if(not GAMESTATE:IsPlayerEnabled(PLAYER_2));
		self:x(WideScale(math.floor(SCREEN_CENTER_X * 1.7) + 8, math.floor(SCREEN_CENTER_X * 1.5) + 8));
		self:y(SCREEN_CENTER_Y-34);
	end;
	CreateStats( PLAYER_2 );
};
return t
