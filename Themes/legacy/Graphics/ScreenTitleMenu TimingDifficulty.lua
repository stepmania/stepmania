local label_text= false
return Def.ActorFrame {
	LoadFont("Common Normal") .. {
		Text=GetTimingDifficulty();
		AltText="";
		InitCommand=cmd(horizalign,left;zoom,0.675);
		OnCommand= function(self)
			label_text= self
			self:shadowlength(1):settextf(Screen.String("TimingDifficulty"), "");
		end,
	};
	LoadFont("Common Normal") .. {
		Text=GetTimingDifficulty();
		AltText="";
		InitCommand=cmd(x,136;zoom,0.675;halign,0);
		OnCommand=function(self)
			self:shadowlength(1):skewx(-0.125):x(label_text:GetZoomedWidth()+8)
			if GetTimingDifficulty() == 9 then
				self:settext(Screen.String("Hardest Timing"));
				(cmd(zoom,0.5;diffuse,ColorLightTone( Color("Orange")) ))(self);
			else
				self:settext( GetTimingDifficulty() );
			end
		end;
	};
};
