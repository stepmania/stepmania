return Def.ActorFrame {
	LoadFont("Common Normal") .. {
		Text=GetLifeDifficulty();
		AltText="";
		InitCommand=function(self)
			self:horizalign(left);
			self:zoom(0.675);
		end;
		OnCommand=function(self)
			self:shadowlength(1);
		end;
		BeginCommand=function(self)
			self:settextf( Screen.String("LifeDifficulty"), "" );
		end
	};
	LoadFont("Common Normal") .. {
		Text=GetLifeDifficulty();
		AltText="";
		InitCommand=function(self)
			self:x(136);
			self:zoom(0.675);
			self:halign(0);
		end;
		OnCommand=function(self)
			self:shadowlength(1);
			self:skewx(-0.125);
		end;
	};
};