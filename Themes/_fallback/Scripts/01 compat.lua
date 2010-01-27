--[[ sm-ssc compatibility helpers
sm-ssc changes quite a few things which would make various SM4 content break.
Also, certain things are deprecated/removed from sm-ssc.
--]]

--[[ Actor ]]
function Actor:hidden(bHide)
	Warn("hidden is deprecated, use visible instead. (used on ".. self:GetName() ..")");
	self:visible(not bHide);
end;

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