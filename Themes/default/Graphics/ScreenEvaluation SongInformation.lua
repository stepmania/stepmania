return Def.ActorFrame {
 	LoadFont("Common Normal") .. {
		Name="TextTitle";
		InitCommand=function(self)
			self:y(-16.5);
			self:zoom(0.875);
			self:maxwidth(256/0.875);
		end;
		OnCommand=function(self)
			self:shadowlength(1);
		end;
	};
 	LoadFont("Common Normal") .. {
		Name="TextSubtitle";
		InitCommand=function(self)
			self:zoom(0.5);
			self:maxwidth(256/0.5);
		end;
		OnCommand=function(self)
			self:shadowlength(1);
		end;
	};
	LoadFont("Common Normal") .. {
		Name="TextArtist";
		InitCommand=function(self)
			self:y(18);
			self:zoom(0.75);
			self:maxwidth(256/0.75);
		end;
		OnCommand=function(self)
			self:shadowlength(1);
			self:skewx(-0.2);
		end;
	};
};