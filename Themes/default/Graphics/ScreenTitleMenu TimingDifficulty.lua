return Def.ActorFrame {
	LoadFont("Common Normal") .. {
		Text=GetTimingDifficulty();
		AltText="";
		InitCommand=function(self)
			self:horizalign(left);
			self:zoom(0.675);
		end;
		OnCommand=function(self)
			self:shadowlength(1);
		end;
		BeginCommand=function(self)
			self:settextf( Screen.String("TimingDifficulty"), "" );
		end
	};
	LoadFont("Common Normal") .. {
		Text=GetTimingDifficulty();
		AltText="";
		InitCommand=function(self)
			self:x(136);
			self:zoom(0.675);
			self:halign(0);
		end;
		OnCommand=function(self)
			self:shadowlength(1);
			self:skewx(-0.125);
			if GetTimingDifficulty() == 9 then
				self:settext("Justice");
				self:zoom(0.5);
				self:diffuse(ColorLightTone( Color("Orange")) );
			else
				self:settext( GetTimingDifficulty() );
			end
		end;
	};
};