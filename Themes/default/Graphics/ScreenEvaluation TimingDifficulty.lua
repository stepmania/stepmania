return LoadFont("_roboto condensed 24px") .. {
	Text=GetLifeDifficulty();
	AltText="";
	BeginCommand=function(self)
		self:settextf( Screen.String("TimingDifficulty"), GetTimingDifficulty() );
		self:diffuse(color("#512232")):zoom(0.8);
		self:diffusealpha(0):sleep(0.5):smooth(0.3):diffusealpha(1);
	end;
};
