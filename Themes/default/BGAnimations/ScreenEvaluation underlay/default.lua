local t = Def.ActorFrame {};

-- A very useful table...
local eval_lines = {
	"W1",
	"W2",
	"W3",
	"W4",
	"W5",
	"Miss",
	"MaxCombo"
}

local eval_radar = {
	Types = { 'Holds', 'Rolls', 'Hands', 'Mines', 'Lifts' },
}

local grade_area_offset = 16
local fade_out_speed = 0.3
local fade_out_pause = 0.08

-- And a function to make even better use out of the table.
local function GetJLineValue(line, pl)
	if line == "Held" then
		return STATSMAN:GetCurStageStats():GetPlayerStageStats(pl):GetHoldNoteScores("HoldNoteScore_Held")
	elseif line == "MaxCombo" then
		return STATSMAN:GetCurStageStats():GetPlayerStageStats(pl):MaxCombo()
	else
		return STATSMAN:GetCurStageStats():GetPlayerStageStats(pl):GetTapNoteScores("TapNoteScore_" .. line)
	end
	return "???"
end

-- You know what, we'll deal with getting the overall scores with a function too.
local function GetPlScore(pl, scoretype)
	local primary_score = STATSMAN:GetCurStageStats():GetPlayerStageStats(pl):GetScore()
	local secondary_score = FormatPercentScore(STATSMAN:GetCurStageStats():GetPlayerStageStats(pl):GetPercentDancePoints())
	
	if PREFSMAN:GetPreference("PercentageScoring") then
		primary_score, secondary_score = secondary_score, primary_score
	end
	
	if scoretype == "primary" then
		return primary_score
	else
		return secondary_score
	end
end

-- #################################################
-- That's enough functions; let's get this done.

-- Shared portion.
local mid_pane = Def.ActorFrame {
	OnCommand=cmd(diffusealpha,0;sleep,0.3;decelerate,0.4;diffusealpha,1);
	OffCommand=cmd(decelerate,0.3;diffusealpha,0);
	-- Song/course banner.
	Def.Sprite {
		InitCommand=function(self)
			local target = GAMESTATE:IsCourseMode() and GAMESTATE:GetCurrentCourse() or GAMESTATE:GetCurrentSong()
			if target and target:HasBanner() then
				self:Load(target:GetBannerPath())
			else
				self:Load(THEME:GetPathG("Common fallback", "banner"))
			end
			self:scaletoclipped(468,146):x(_screen.cx):y(_screen.cy-173):zoom(0.8)
		end
	},
	-- Banner frame.
	LoadActor("_bannerframe") .. {
		InitCommand=cmd(x,_screen.cx;y,_screen.cy-172;zoom,0.8)
	}
}

-- Song or Course Title
if not GAMESTATE:IsCourseMode() then
	mid_pane[#mid_pane+1] = Def.BitmapText {
		Font="Common Fallback",
		InitCommand=function(self)
			self:x(_screen.cx):y(_screen.cy+188-6):diffuse(color("#512232")):shadowlength(1):zoom(0.75):maxwidth(500)
		end;
		OnCommand=function(self)
			local song = GAMESTATE:GetCurrentSong();
			if song then
				self:settext(song:GetDisplayMainTitle());
			else
				self:settext("");
			end;
			self:diffusealpha(0):sleep(1.0):decelerate(0.4):diffusealpha(1)
		end,
		OffCommand=cmd(decelerate,0.4;diffusealpha,0)
	}
	mid_pane[#mid_pane+1] = Def.BitmapText {
		Font="Common Fallback",
		InitCommand=function(self)
			self:x(_screen.cx):y(_screen.cy+188+22-6):diffuse(color("#512232")):shadowlength(1):zoom(0.6):maxwidth(500)
		end;
		OnCommand=function(self)
			local song = GAMESTATE:GetCurrentSong();
			if song then
				self:settext(song:GetDisplaySubTitle());
			else
				self:settext("");
			end;
			self:diffusealpha(0):sleep(1.1):decelerate(0.4):diffusealpha(1)
		end,
		OffCommand=cmd(decelerate,0.4;diffusealpha,0)
	}
else
	mid_pane[#mid_pane+1] = Def.BitmapText {
		Font="Common Fallback",
		InitCommand=function(self)
			self:x(_screen.cx):y(_screen.cy+188-6):diffuse(color("#512232")):shadowlength(1):zoom(0.75):maxwidth(500)
		end;
		OnCommand=function(self)
			local course = GAMESTATE:GetCurrentCourse()
			self:settext(course:GetDisplayFullTitle())
			self:diffusealpha(0):sleep(1.3):decelerate(0.4):diffusealpha(1)
		end,
		OffCommand=cmd(decelerate,0.4;diffusealpha,0)
	}
end

-- Each line's text, and associated decorations.
for i, v in ipairs(eval_lines) do
	local spacing = 38*i
	local cur_line = "JudgmentLine_" .. v
	
	mid_pane[#mid_pane+1] = Def.ActorFrame{
		InitCommand=cmd(x,_screen.cx;y,(_screen.cy/1.48)+(spacing)),
		Def.Quad {
			InitCommand=cmd(zoomto,400,36;diffuse,JudgmentLineToColor(cur_line);fadeleft,0.5;faderight,0.5);
			OnCommand=function(self)			
				self:diffusealpha(0):sleep(0.1 * i):decelerate(0.6):diffusealpha(1)
			end;
			OffCommand=function(self)			
				self:sleep(fade_out_pause * i):decelerate(fade_out_speed):diffusealpha(0)
			end;				
		};
	
		Def.BitmapText {
			Font = "_roboto condensed Bold 48px",
			InitCommand=cmd(zoom,0.6;diffuse,color("#000000");settext,string.upper(JudgmentLineToLocalizedString(cur_line)));
			OnCommand=function(self)			
				self:diffusealpha(0):sleep(0.1 * i):decelerate(0.6):diffusealpha(0.8)
			end;
			OffCommand=function(self)			
				self:sleep(fade_out_pause * i):decelerate(fade_out_speed):diffusealpha(0)
			end;	
		}
	}
end

t[#t+1] = mid_pane

-- #################################################
-- Time to deal with all of the player stats. ALL OF THEM.

local eval_parts = Def.ActorFrame {}

for ip, p in ipairs(GAMESTATE:GetHumanPlayers()) do
	-- Some things to help positioning
	local step_count_offs = string.find(p, "P1") and -140 or 140
	local grade_parts_offs = string.find(p, "P1") and -320 or 320
	local p_grade = STATSMAN:GetCurStageStats():GetPlayerStageStats(p):GetGrade()
	
	-- Step counts.
	for i, v in ipairs(eval_lines) do
		local spacing = 38*i
		eval_parts[#eval_parts+1] = Def.BitmapText {
			Font = "_overpass 36px",
			InitCommand=cmd(x,_screen.cx + step_count_offs;y,(_screen.cy/1.48)+(spacing);diffuse,ColorDarkTone(PlayerColor(p));zoom,0.75;diffusealpha,1.0;shadowlength,1;maxwidth,120),
			OnCommand=function(self)
				self:settext(GetJLineValue(v, p))
				if string.find(p, "P1") then
					self:horizalign(right)
				else
					self:horizalign(left)
				end
				self:diffusealpha(0):sleep(0.1 * i):decelerate(0.6):diffusealpha(1)
			end;
			OffCommand=function(self)			
				self:sleep(fade_out_pause * i):decelerate(fade_out_speed):diffusealpha(0)
			end;	
		}
	end

	-- Letter grade and associated parts.
	eval_parts[#eval_parts+1] = Def.ActorFrame{
		InitCommand=cmd(x,_screen.cx + grade_parts_offs;y,_screen.cy/1.91),
		
		--Containers
		Def.Quad {
			InitCommand=cmd(zoomto,190,115;diffuse,ColorLightTone(PlayerColor(p));diffusebottomedge,color("#FEEFCA")),
			OnCommand=function(self)
			    self:diffusealpha(0):decelerate(0.4):diffusealpha(0.5)
			end,
			OffCommand=cmd(decelerate,0.3;diffusealpha,0)
		},
		
		Def.Quad {
			InitCommand=cmd(vertalign,top;y,60+grade_area_offset;zoomto,190,136;diffuse,color("#fce1a1")),
			OnCommand=function(self)
			    self:diffusealpha(0):decelerate(0.4):diffusealpha(0.4)
			end,
			OffCommand=cmd(decelerate,0.3;diffusealpha,0)
		},
		
		LoadActor(THEME:GetPathG("GradeDisplay", "Grade " .. p_grade)) .. {
			InitCommand=cmd(zoom,0.75);
			OnCommand=function(self)
			        self:diffusealpha(0):zoom(1):sleep(0.63):decelerate(0.4):zoom(0.75):diffusealpha(1)
					if STATSMAN:GetCurStageStats():GetPlayerStageStats(p):GetStageAward() then
					  self:sleep(0.1):decelerate(0.4):addy(-12);
					else
					  self:addy(0);
					end;
			end;
			OffCommand=cmd(decelerate,0.3;diffusealpha,0);			
		},
		
		Def.BitmapText {
			Font = "_roboto condensed 24px",
			InitCommand=cmd(diffuse,Color.White;zoom,1;addy,38;maxwidth,160;uppercase,true;diffuse,ColorDarkTone(PlayerDarkColor(p));diffusetopedge,ColorMidTone(PlayerColor(p));shadowlength,1),
			OnCommand=function(self)
				if STATSMAN:GetCurStageStats():GetPlayerStageStats(p):GetStageAward() then
					self:settext(THEME:GetString( "StageAward", ToEnumShortString(STATSMAN:GetCurStageStats():GetPlayerStageStats(p):GetStageAward()) ))
					self:diffusealpha(0):zoomx(0.5):sleep(1):decelerate(0.4):zoomx(1):diffusealpha(1)
				end
			end;
			OffCommand=cmd(decelerate,0.3;diffusealpha,0);
		}
	}
	
		
	-- Primary score.
	eval_parts[#eval_parts+1] = Def.BitmapText {
		Font = "_overpass 36px",
		InitCommand=cmd(horizalign,center;x,_screen.cx + (grade_parts_offs);y,(_screen.cy-59)+grade_area_offset;diffuse,ColorMidTone(PlayerColor(p));zoom,1;shadowlength,1;maxwidth,180),
		OnCommand=function(self)
			self:settext(GetPlScore(p, "primary")):diffusealpha(0):sleep(0.5):decelerate(0.3):diffusealpha(1)
		end;
		OffCommand=function(self)
			self:decelerate(0.3):diffusealpha(0)
		end;
	}
	-- Secondary score.
	eval_parts[#eval_parts+1] = Def.BitmapText {
		Font = "_overpass 36px",
		InitCommand=cmd(horizalign,center;x,_screen.cx + (grade_parts_offs);y,(_screen.cy-59)+35+grade_area_offset;diffuse,ColorDarkTone(PlayerColor(p));zoom,0.75;shadowlength,1),
		OnCommand=function(self)
			self:settext(GetPlScore(p, "secondary")):diffusealpha(0):sleep(0.6):decelerate(0.3):diffusealpha(1)
		end;
		OffCommand=function(self)
			self:sleep(0.1):decelerate(0.3):diffusealpha(0)
		end;
	}
	
	eval_parts[#eval_parts+1] = Def.BitmapText {
		Font = "Common Condensed",
		InitCommand=cmd(horizalign,center;x,_screen.cx + (grade_parts_offs);y,(_screen.cy-50)+56+grade_area_offset;diffuse,ColorDarkTone(PlayerColor(p));zoom,0.75;shadowlength,1;maxwidth,180),
		OnCommand=function(self)
			local record = STATSMAN:GetCurStageStats():GetPlayerStageStats(p):GetPersonalHighScoreIndex()
			local hasPersonalRecord = record ~= -1
			self:visible(hasPersonalRecord);
			local text = string.format(THEME:GetString("ScreenEvaluation", "PersonalRecord"), record+1)
			self:settext(text)
			self:diffusealpha(0):sleep(0.6):decelerate(0.3):diffusealpha(0.9)
		end;
		OffCommand=function(self)
			self:sleep(0.1):decelerate(0.3):diffusealpha(0)
		end;
	}
	
	-- Other stats (holds, mines, etc.)
	for i, rc_type in ipairs(eval_radar.Types) do
		local performance = STATSMAN:GetCurStageStats():GetPlayerStageStats(p):GetRadarActual():GetValue( "RadarCategory_"..rc_type )
		local possible = STATSMAN:GetCurStageStats():GetPlayerStageStats(p):GetRadarPossible():GetValue( "RadarCategory_"..rc_type )
		local label = THEME:GetString("RadarCategory", rc_type)
	
		eval_parts[#eval_parts+1] = Def.ActorFrame {
			InitCommand=function(self)
				self:x(_screen.cx + (grade_parts_offs))
				self:y((_screen.cy + 104 - 32) + (i-1)*32)
			end;
			OnCommand=function(self)			
				self:diffusealpha(0):sleep(0.1 * i):decelerate(0.5):diffusealpha(1)
			end;
			OffCommand=function(self)			
			self:sleep(0.13 * i):decelerate(0.6):diffusealpha(0)
			end;	
				Def.Quad {
					InitCommand=cmd(zoomto,190,28;diffuse,color("#fce1a1");diffusealpha,0.4);
				};
				Def.BitmapText {
					Font = "Common Condensed",
					InitCommand=cmd(zoom,0.8;x,-80;horizalign,left;diffuse,color("0,0,0,0.75");shadowlength,1),
					BeginCommand=function(self)
						self:settext(label .. ":")
					end
				};
				Def.BitmapText {
					Font = "_overpass 36px",
					InitCommand=cmd(zoom,0.5;x,83;horizalign,right;maxwidth,200;diffuse,ColorDarkTone(PlayerColor(p));shadowlength,1),
					BeginCommand=function(self)
						self:settext(performance .. "/" .. possible)
					end
				};
		};
	end;
	
	-- Options
	eval_parts[#eval_parts+1] = Def.BitmapText {
		Font = "Common Condensed",
		InitCommand=cmd(horizalign,center;vertalign,top;x,_screen.cx + (grade_parts_offs);y,(_screen.cy+196+43);wrapwidthpixels,240;diffuse,ColorDarkTone(PlayerColor(p));zoom,0.75;shadowlength,1),
		OnCommand=function(self)
			self:settext(GAMESTATE:GetPlayerState(p):GetPlayerOptionsString(0))
			self:diffusealpha(0):sleep(0.8):decelerate(0.6):diffusealpha(1)
			end;				
		OffCommand=function(self)
			self:sleep(0.1):decelerate(0.3):diffusealpha(0)
		end;
		};
end

t[#t+1] = eval_parts


-- todo: replace.
if GAMESTATE:IsHumanPlayer(PLAYER_1) == true then
	if GAMESTATE:IsCourseMode() == false then
	-- Difficulty banner
	local grade_parts_offs = -320
	t[#t+1] = Def.ActorFrame {
	  InitCommand=cmd(horizalign,center;x,_screen.cx + grade_parts_offs;y,_screen.cy-96+grade_area_offset;visible,not GAMESTATE:IsCourseMode());
	  OnCommand=cmd(zoomx,0.3;diffusealpha,0;sleep,0.5;decelerate,0.4;zoomx,1;diffusealpha,1);
	  OffCommand=cmd(decelerate,0.4;diffusealpha,0);
	  LoadFont("Common Fallback") .. {
			InitCommand=cmd(zoom,1;horizalign,center;shadowlength,1);
			OnCommand=cmd(playcommand,"Set");
			CurrentStepsP1ChangedMessageCommand=cmd(playcommand,"Set");
			ChangedLanguageDisplayMessageCommand=cmd(playcommand,"Set");
			SetCommand=function(self)
			local stepsP1 = GAMESTATE:GetCurrentSteps(PLAYER_1)
			local song = GAMESTATE:GetCurrentSong();
			  if song then
				if stepsP1 ~= nil then
				  local st = stepsP1:GetStepsType();
				  local diff = stepsP1:GetDifficulty();
				  local courseType = GAMESTATE:IsCourseMode() and SongOrCourse:GetCourseType() or nil;
				  local cdp1 = GetCustomDifficulty(st, diff, courseType);
				  self:settext(string.upper(THEME:GetString("CustomDifficulty",ToEnumShortString(diff))) .. "  " .. stepsP1:GetMeter());
				  self:diffuse(ColorDarkTone(CustomDifficultyToColor(cdp1)));				  
				else
				  self:settext("")
				end
			  else
				self:settext("")
			  end
			end
		};
	  };
	end;

end;


if GAMESTATE:IsHumanPlayer(PLAYER_2) == true then

	if GAMESTATE:IsCourseMode() == false then
	local grade_parts_offs = 320	
	t[#t+1] = Def.ActorFrame {
	  InitCommand=cmd(horizalign,center;x,_screen.cx + grade_parts_offs;y,_screen.cy-96+grade_area_offset;visible,not GAMESTATE:IsCourseMode());
	  OnCommand=cmd(zoomx,0.3;diffusealpha,0;sleep,0.5;decelerate,0.4;zoomx,1;diffusealpha,1);
	  OffCommand=cmd(decelerate,0.4;diffusealpha,0);
	  LoadFont("Common Fallback") .. {
			InitCommand=cmd(zoom,1;horizalign,center;shadowlength,1);
			OnCommand=cmd(playcommand,"Set");
			CurrentStepsP2ChangedMessageCommand=cmd(playcommand,"Set");
			ChangedLanguageDisplayMessageCommand=cmd(playcommand,"Set");
			SetCommand=function(self)
			local stepsP2 = GAMESTATE:GetCurrentSteps(PLAYER_2)
			local song = GAMESTATE:GetCurrentSong();
			  if song then
				if stepsP2 ~= nil then
				  local st = stepsP2:GetStepsType();
				  local diff = stepsP2:GetDifficulty();
				  local courseType = GAMESTATE:IsCourseMode() and SongOrCourse:GetCourseType() or nil;
				  local cdp2 = GetCustomDifficulty(st, diff, courseType);
				  self:settext(string.upper(THEME:GetString("CustomDifficulty",ToEnumShortString(diff))) .. "  " .. stepsP2:GetMeter());
				  self:diffuse(ColorDarkTone(CustomDifficultyToColor(cdp2)));				  
				else
				  self:settext("")
				end
			  else
				self:settext("")
			  end
			end
		};
	  };
	  
	 end; 

end;

t[#t+1] = StandardDecorationFromFileOptional("LifeDifficulty","LifeDifficulty");
t[#t+1] = StandardDecorationFromFileOptional("TimingDifficulty","TimingDifficulty");

if gameplay_pause_count > 0 then
	t[#t+1]= Def.BitmapText{
		Font= "Common Italic Condensed",
		Text= THEME:GetString("PauseMenu", "pause_count") .. ": " .. gameplay_pause_count,
		InitCommand=cmd(x,SCREEN_CENTER_X;y,SCREEN_CENTER_Y-130;shadowlength,1;maxwidth,140);
		OnCommand=function(self)
			self:diffuse(color("#FF0000")):diffusebottomedge(color("#512232")):zoom(0.8);
			self:diffusealpha(0):sleep(1.5):smooth(0.3):diffusealpha(1);
		end;
		OffCommand=cmd(sleep,0.2;decelerate,0.3;diffusealpha,0);
	}
end

return t;