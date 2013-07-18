return Def.ActorFrame {
	LoadFont("Common Normal") .. {
		Text=GetLifeDifficulty();
		AltText="";
		InitCommand=cmd(horizalign,left;zoom,0.675);
		OnCommand=cmd(shadowlength,1);
		BeginCommand=function(self)
			self:settextf( Screen.String("LifeDifficulty"), "" );
		end
	};
	LoadFont("Common Normal") .. {
		Text=GetLifeDifficulty();
		AltText="";
		InitCommand=cmd(x,136;zoom,0.675;halign,0);
		OnCommand=cmd(shadowlength,1;skewx,-0.125);
	};
};