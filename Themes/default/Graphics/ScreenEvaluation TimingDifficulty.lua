return LoadFont("Common Normal") .. {
	Text=GetLifeDifficulty();
	AltText="";
	BeginCommand=function(self)
		self:settextf( Screen.String("TimingDifficulty"), GetTimingDifficulty() );
	end
};