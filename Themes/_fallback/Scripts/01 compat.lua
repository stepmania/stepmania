--[[ sm-ssc compatibility helpers
sm-ssc changes quite a few things which would make various SM4 content break.
Also, certain things are deprecated/removed from sm-ssc (and sometimes SM4 too).
--]]

--[[ Actor ]]
function Actor:hidden(bHide)
	Warn("hidden is deprecated, use visible instead. (used on ".. self:GetName() ..")");
	self:visible(not bHide);
end;

-- for when horizalign and vertalign get killed:
--[[
function Actor:horizalign(v)
	local values = {
		left = 0;
		center = 0.5;
		right = 1;
	};
	self:halign(values[v]);
end;

function Actor:vertalign(v)
	local values = {
		top = 0;
		middle = 0.5;
		bottom = 1;
	};
	self:valign(values[v]);
end;
--]]

--[[ ActorScroller: all of these got renamed, so alias the lowercase ones if
things are going to look for them. ]]
function ActorScroller:getsecondtodestination()
	self:GetSecondsToDestination();
end;

function ActorScroller:setsecondsperitem(secs)
	self:SetSecondsPerItem(secs);
end;

function ActorScroller:setnumsubdivisions(subs)
	self:SetNumSubdivisions(subs);
end;

function ActorScroller:scrollthroughallitems()
	self:ScrollThroughAllItems();
end;

function ActorScroller:scrollwithpadding(fPadStart,fPadEnd)
	self:ScrollWithPadding(fPadStart,fPadEnd);
end;

function ActorScroller:setfastcatchup(bFastCatchup)
	self:SetFastCatchup(bFastCatchup);
end;

-- renaming various StepMania functions to sm-ssc ones:
if ScreenString then ScreenString = Screen.String; end;
if ScreenMetric then ScreenMetric = Screen.Metric; end;

-- SM4SVN r28330: "add IsFullCombo bindings" - Chris Danford
-- Unfortunately we already bound FullComboOfScore(tns), meaning we're not
-- going to go back and redefine bindings that match it. Alias them instead
-- for any themes expecting to use them:
function PlayerStageStats:IsFullComboW1()
	return self:FullComboOfScore('TapNoteScore_W1');
end;

function PlayerStageStats:IsFullComboW2()
	return self:FullComboOfScore('TapNoteScore_W2');
end;

function PlayerStageStats:IsFullComboW3()
	return self:FullComboOfScore('TapNoteScore_W3');
end;

function PlayerStageStats:IsFullComboW4()
	return self:FullComboOfScore('TapNoteScore_W4');
end;