local frame = Def.ActorFrame
{
	OnCommand=cmd(x,SCREEN_CENTER_X+180;y,SCREEN_CENTER_Y);--;addx,SCREEN_WIDTH/2;decelerate,0.5;addx,-SCREEN_WIDTH/2);
	OffCommand=cmd(accelerate,0.5;addx,SCREEN_WIDTH/2);
	LoadFont("Common", "normal") .. {
		OnCommand=cmd(horizalign,left;x,-80;y,-130;playcommand,"CurrentCourseChanged");
		CurrentCourseChangedMessageCommand=function(self)
			local c = GAMESTATE:GetCurrentCourse()
			if c then
				self:settext( c:GetDisplayFullTitle() )
			else
				self:settext( "" )
			end
		end;
	};
	LoadFont("Common", "normal") .. {
		OnCommand=cmd(horizalign,left;x,-80;y,-100;playcommand,"CurrentTrailP1Changed");
		CurrentTrailP1ChangedMessageCommand=function(self)
			local t = GAMESTATE:GetCurrentTrail( PLAYER_1 )
			if t then
				local st = GameManager.StepsTypeToLocalizedString( t:GetStepsType() )
				local cd = CourseDifficultyToLocalizedString( t:GetDifficulty() )
				self:settext( st .. ", " .. cd )
			else
				self:settext( "" )
			end
		end;
	};
};

return Def.ActorFrame { frame };
