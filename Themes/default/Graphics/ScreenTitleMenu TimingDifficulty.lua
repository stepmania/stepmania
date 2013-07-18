return Def.ActorFrame {
	LoadFont("Common Normal") .. {
		Text=GetTimingDifficulty();
		AltText="";
		InitCommand=cmd(horizalign,left;zoom,0.675);
		OnCommand=cmd(shadowlength,1);
		BeginCommand=function(self)
			self:settextf( Screen.String("TimingDifficulty"), "" );
		end
	};
	LoadFont("Common Normal") .. {
		Text=GetTimingDifficulty();
		AltText="";
		InitCommand=cmd(x,136;zoom,0.675;halign,0);
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