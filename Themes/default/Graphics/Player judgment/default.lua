local c;
local player = Var "Player";
local function ShowProtiming()
  if GAMESTATE:IsDemonstration() then
    return false
  else
    return GetUserPrefB("UserPrefProtiming" .. ToEnumShortString(player));
  end
end;
local bShowProtiming = ShowProtiming();
local ProtimingWidth = 240;
local function MakeAverage( t )
	local sum = 0;
	for i=1,#t do
		sum = sum + t[i];
	end
	return sum / #t
end

local tTotalJudgments = {};

local JudgeCmds = {
	TapNoteScore_W1 = THEME:GetMetric( "Judgment", "JudgmentW1Command" );
	TapNoteScore_W2 = THEME:GetMetric( "Judgment", "JudgmentW2Command" );
	TapNoteScore_W3 = THEME:GetMetric( "Judgment", "JudgmentW3Command" );
	TapNoteScore_W4 = THEME:GetMetric( "Judgment", "JudgmentW4Command" );
	TapNoteScore_W5 = THEME:GetMetric( "Judgment", "JudgmentW5Command" );
	TapNoteScore_Miss = THEME:GetMetric( "Judgment", "JudgmentMissCommand" );
};

local ProtimingCmds = {
	TapNoteScore_W1 = THEME:GetMetric( "Protiming", "ProtimingW1Command" );
	TapNoteScore_W2 = THEME:GetMetric( "Protiming", "ProtimingW2Command" );
	TapNoteScore_W3 = THEME:GetMetric( "Protiming", "ProtimingW3Command" );
	TapNoteScore_W4 = THEME:GetMetric( "Protiming", "ProtimingW4Command" );
	TapNoteScore_W5 = THEME:GetMetric( "Protiming", "ProtimingW5Command" );
	TapNoteScore_Miss = THEME:GetMetric( "Protiming", "ProtimingMissCommand" );
};

local AverageCmds = {
	Pulse = THEME:GetMetric( "Protiming", "AveragePulseCommand" );
};
local TextCmds = {
	Pulse = THEME:GetMetric( "Protiming", "TextPulseCommand" );
};

local TNSFrames = {
	TapNoteScore_W1 = 0;
	TapNoteScore_W2 = 1;
	TapNoteScore_W3 = 2;
	TapNoteScore_W4 = 3;
	TapNoteScore_W5 = 4;
	TapNoteScore_Miss = 5;
};
local t = Def.ActorFrame {};
t[#t+1] = Def.ActorFrame {
	LoadActor(THEME:GetPathG("Judgment","Normal")) .. {
		Name="Judgment";
		InitCommand=function(self)
			self:pause();
			self:visible(false);
		end;
		OnCommand=THEME:GetMetric("Judgment","JudgmentOnCommand");
		ResetCommand=function(self)
			self:finishtweening();
			self:stopeffect();
			self:visible(false);
		end;
	};
	LoadFont("Combo Numbers") .. {
		Name="ProtimingDisplay";
		Text="";
		InitCommand=function(self)
			self:visible(false);
		end;
		OnCommand=THEME:GetMetric("Protiming","ProtimingOnCommand");
		ResetCommand=function(self)
			self:finishtweening();
			self:stopeffect();
			self:visible(false);
		end;
	};
	LoadFont("Common Normal") .. {
		Name="ProtimingAverage";
		Text="";
		InitCommand=function(self)
			self:visible(false);
		end;
		OnCommand=THEME:GetMetric("Protiming","AverageOnCommand");
		ResetCommand=function(self)
			self:finishtweening();
			self:stopeffect();
			self:visible(false);
		end;
	};
	LoadFont("Common Normal") .. {
		Name="TextDisplay";
		Text=THEME:GetString("Protiming","MS");
		InitCommand=function(self)
			self:visible(false);
		end;
		OnCommand=THEME:GetMetric("Protiming","TextOnCommand");
		ResetCommand=function(self)
			self:finishtweening();
			self:stopeffect();
			self:visible(false);
		end;
	};
	Def.Quad {
		Name="ProtimingGraphBG";
		InitCommand=function(self)
			self:visible(false);
			self:y(32);
			self:zoomto(ProtimingWidth, 16);
		end;
		ResetCommand=function(self)
			self:finishtweening();
			self:diffusealpha(0.8);
			self:visible(false);
		end;
		OnCommand=function(self)
			self:diffuse(Color("Black"));
			self:diffusetopedge(color("0.1,0.1,0.1,1"));
			self:diffusealpha(0.8);
			self:shadowlength(2);
		end;
	};
	Def.Quad {
		Name="ProtimingGraphWindowW3";
		InitCommand=function(self)
			self:visible(false);
			self:y(32);
			self:zoomto(ProtimingWidth - 4, 16 - 4);
		end;
		ResetCommand=function(self)
			self:finishtweening();
			self:diffusealpha(1);
			self:visible(false);
		end;
		OnCommand=function(self)
			self:diffuse(GameColor.Judgment["JudgmentLine_W3"]);
		end;
	};
	Def.Quad {
		Name="ProtimingGraphWindowW2";
		InitCommand=function(self)
			self:visible(false);
			self:y(32);
			self:zoomto(scale(
				PREFSMAN:GetPreference("TimingWindowSecondsW2"),
				0,
				PREFSMAN:GetPreference("TimingWindowSecondsW3"),
				0,
				ProtimingWidth-4),16-4);
		end;
		ResetCommand=function(self)
			self:finishtweening();
			self:diffusealpha(1);
			self:visible(false);
		end;
		OnCommand=function(self)
			self:diffuse(GameColor.Judgment["JudgmentLine_W2"]);
		end;
	};
	Def.Quad {
		Name="ProtimingGraphWindowW1";
		InitCommand=function(self)
			self:visible(false);
			self:y(32);
			self:zoomto(scale(
				PREFSMAN:GetPreference("TimingWindowSecondsW1"),
				0,
				PREFSMAN:GetPreference("TimingWindowSecondsW3"),
				0,
				ProtimingWidth-4),16-4);
		end;
		ResetCommand=function(self)
			self:finishtweening();
			self:diffusealpha(1);
			self:visible(false);
		end;
		OnCommand=function(self)
			self:diffuse(GameColor.Judgment["JudgmentLine_W1"]);
		end;
	};
	Def.Quad {
		Name="ProtimingGraphUnderlay";
		InitCommand=function(self)
			self:visible(false);
			self:y(32);
			self:zoomto(ProtimingWidth-4,16-4);
		end;
		ResetCommand=function(self)
			self:finishtweening();
			self:diffusealpha(0.25);
			self:visible(false);
		end;
		OnCommand=function(self)
			self:diffuse(Color("Black"));
			self:diffusealpha(0.25);
		end;
	};
	Def.Quad {
		Name="ProtimingGraphFill";
		InitCommand=function(self)
			self:visible(false);
			self:y(32);
			self:zoomto(0,16-4);
			self:horizalign(left);
		end;
		ResetCommand=function(self)
			self:finishtweening();
			self:diffusealpha(1);
			self:visible(false);
		end;
		OnCommand=function(self)
			self:diffuse(Color("Red"));
		end;
	};
	Def.Quad {
		Name="ProtimingGraphAverage";
		InitCommand=function(self)
			self:visible(false);
			self:y(32);
			self:zoomto(2,7);
		end;
		ResetCommand=function(self)
			self:finishtweening();
			self:diffusealpha(0.85);
			self:visible(false);
		end;
		OnCommand=function(self)
			self:diffuse(Color("Orange"));
			self:diffusealpha(0.85);
		end;
	};
	Def.Quad {
		Name="ProtimingGraphCenter";
		InitCommand=function(self)
			self:visible(false);
			self:y(32);
			self:zoomto(2,16-4);
		end;
		ResetCommand=function(self)
			self:finishtweening();
			self:diffusealpha(1);
			self:visible(false);
		end;
		OnCommand=function(self)
			self:diffuse(Color("White"));
			self:diffusealpha(1);
		end;
	};
	InitCommand = function(self)
		c = self:GetChildren();
	end;

	JudgmentMessageCommand=function(self, param)
        -- Fix Player Combo animating when player successfully avoids a mine.
        local msgParam = param;
        MESSAGEMAN:Broadcast("TestJudgment",msgParam);
        --
		if param.Player ~= player then return end;
		if param.HoldNoteScore then return end;
		
		local iNumStates = c.Judgment:GetNumStates();
		local iFrame = TNSFrames[param.TapNoteScore];
		
		if not iFrame then return end
		if iNumStates == 12 then
			iFrame = iFrame * 2;
			if not param.Early then
				iFrame = iFrame + 1;
			end
		end
		

		local fTapNoteOffset = param.TapNoteOffset;
		if param.HoldNoteScore then
			fTapNoteOffset = 1;
		else
			fTapNoteOffset = param.TapNoteOffset; 
		end
		
		if param.TapNoteScore == 'TapNoteScore_Miss' then
			fTapNoteOffset = 1;
			bUseNegative = true;
		else
-- 			fTapNoteOffset = fTapNoteOffset;
			bUseNegative = false;
		end;
		
		if fTapNoteOffset ~= 1 then
			-- we're safe, you can push the values
			tTotalJudgments[#tTotalJudgments+1] = math.abs(fTapNoteOffset);
--~ 			tTotalJudgments[#tTotalJudgments+1] = bUseNegative and fTapNoteOffset or math.abs( fTapNoteOffset );
		end
		
		self:playcommand("Reset");

		c.Judgment:visible( not bShowProtiming );
		c.Judgment:setstate( iFrame );
		JudgeCmds[param.TapNoteScore](c.Judgment);
		
		c.ProtimingDisplay:visible( bShowProtiming );
		c.ProtimingDisplay:settextf("%i",fTapNoteOffset * 1000);
		ProtimingCmds[param.TapNoteScore](c.ProtimingDisplay);
		
		c.ProtimingAverage:visible( bShowProtiming );
		c.ProtimingAverage:settextf("%.2f%%",clamp(100 - MakeAverage( tTotalJudgments ) * 1000 ,0,100));
		AverageCmds['Pulse'](c.ProtimingAverage);
		
		c.TextDisplay:visible( bShowProtiming );
		TextCmds['Pulse'](c.TextDisplay);
		
		c.ProtimingGraphBG:visible( bShowProtiming );
		c.ProtimingGraphUnderlay:visible( bShowProtiming );
		c.ProtimingGraphWindowW3:visible( bShowProtiming );
		c.ProtimingGraphWindowW2:visible( bShowProtiming );
		c.ProtimingGraphWindowW1:visible( bShowProtiming );
		c.ProtimingGraphFill:visible( bShowProtiming );
		c.ProtimingGraphFill:finishtweening();
		c.ProtimingGraphFill:decelerate(1/60);
		c.ProtimingGraphFill:zoomtowidth( clamp(
				scale(
				fTapNoteOffset,
				0,PREFSMAN:GetPreference("TimingWindowSecondsW3"),
				0,(ProtimingWidth-4)/2),
			-(ProtimingWidth-4)/2,(ProtimingWidth-4)/2)
		);
		c.ProtimingGraphAverage:visible( bShowProtiming );
		c.ProtimingGraphAverage:zoomtowidth( clamp(
				scale(
				MakeAverage( tTotalJudgments ),
				0,PREFSMAN:GetPreference("TimingWindowSecondsW3"),
				0,ProtimingWidth-4),
			0,ProtimingWidth-4)
		);
-- 		c.ProtimingGraphAverage:zoomtowidth( clamp(MakeAverage( tTotalJudgments ) * 1880,0,188) );
		c.ProtimingGraphCenter:visible( bShowProtiming );
		c.ProtimingGraphBG.sleep(2);
		c.ProtimingGraphBG.linear(0.5);
		c.ProtimingGraphBG.diffusealpha(0);
		c.ProtimingGraphUnderlay.sleep(2);
		c.ProtimingGraphUnderlay.linear(0.5);
		c.ProtimingGraphUnderlay.diffusealpha(0);
		c.ProtimingGraphWindowW3.sleep(2);
		c.ProtimingGraphWindowW3.linear(0.5);
		c.ProtimingGraphWindowW3.diffusealpha(0);
		c.ProtimingGraphWindowW2.sleep(2);
		c.ProtimingGraphWindowW2.linear(0.5);
		c.ProtimingGraphWindowW2.diffusealpha(0);
		c.ProtimingGraphWindowW1.sleep(2);
		c.ProtimingGraphWindowW1.linear(0.5);
		c.ProtimingGraphWindowW1.diffusealpha(0);
		c.ProtimingGraphFill.sleep(2);
		c.ProtimingGraphFill.linear(0.5);
		c.ProtimingGraphFill.diffusealpha(0);
		c.ProtimingGraphAverage.sleep(2);
		c.ProtimingGraphAverage.linear(0.5);
		c.ProtimingGraphAverage.diffusealpha(0);
		c.ProtimingGraphCenter.sleep(2);
		c.ProtimingGraphCenter.linear(0.5);
		c.ProtimingGraphCenter.diffusealpha(0);
	end;

};


return t;
