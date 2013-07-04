return Def.ActorFrame {
	LoadFont("Common Normal") .. {
		Text=string.format("%s %s", ProductFamily(), ProductVersion());
		AltText="StepMania";
		InitCommand=function(self)
			self:zoom(0.675);
		end;
		OnCommand=function(self)
			self:horizalign(right);
			self:shadowlength(1);
		end;
	};
	LoadFont("Common Normal") .. {
		Text=string.format("%s", VersionDate());
		AltText="Unknown Version";
		InitCommand=function(self)
			self:y(16);
			self:zoom(0.5);
		end;
		OnCommand=function(self)
			self:horizalign(right);
			self:shadowlength(1);
		end;
	};
};