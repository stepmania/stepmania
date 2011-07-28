return Def.ActorFrame {
--[[ 	LoadActor(THEME:GetPathG("ScreenTitleMenu","PreferenceFrame")) .. {
		OnCommand=function(self)
			if GetTimingDifficulty() == 7 then
				(cmd(glowshift;diffuse,Color("Yellow");diffuserightedge,Color("Red");effectcolor1,Color.Alpha( Color("Red"), 0.5 );effectcolor2,Color("Invisible");effectperiod,1.25/2))(self);
			elseif GetTimingDifficulty() < 4 then
				(cmd(diffuseshift;effectcolor1,Color("Blue");effectcolor2,ColorDarkTone(Color("Blue"));effectperiod,1.25))(self);
			elseif GetTimingDifficulty() > 4 and GetTimingDifficulty < 6 then
				(cmd(diffuseshift;effectcolor1,Color("Red");effectcolor2,ColorDarkTone(Color("Red"));effectperiod,1.25))(self);
			else
				(cmd(diffuse,Color("Orange");diffusetopedge,Color("Yellow")))(self);
			end;
		end;
	}; --]]
-- 	LoadActor(THEME:GetPathG("OptionRowExit","frame"));
	LoadActor(THEME:GetPathG("_icon","Timing")) .. {
		InitCommand=cmd(x,-60;shadowlength,1);
	};
	LoadFont("Common Normal") .. {
		Text=GetTimingDifficulty();
		AltText="";
		InitCommand=cmd(x,-72+28;horizalign,left;zoom,0.5);
		OnCommand=cmd(shadowlength,1);
		BeginCommand=function(self)
			self:settextf( Screen.String("TimingDifficulty"), "" );
		end
	};
	LoadFont("Common Normal") .. {
		Text=GetTimingDifficulty();
		AltText="";
		InitCommand=cmd(x,72*0.75+8;zoom,0.875);
		OnCommand=function(self)
			(cmd(shadowlength,1;skewx,-0.125))(self);
			if GetTimingDifficulty() == 9 then
				self:settext("Justice");
				(cmd(zoom,0.5;diffuse,ColorLightTone( Color("Orange")) ))(self);
			else
				self:settext( GetTimingDifficulty() );
			end
		end;
	};
};