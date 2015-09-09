local label_text= false

return Def.ActorFrame {
	LoadFont("Common Normal") .. {
		Text=GetLifeDifficulty();
		AltText="";
		InitCommand=cmd(horizalign,left;zoom,0.675);
		OnCommand= function(self)
			label_text= self
			self:shadowlength(1):settextf(Screen.String("LifeDifficulty"), "");
		end,
	};
	LoadFont("Common Normal") .. {
		Text=GetLifeDifficulty();
		AltText="";
		InitCommand=cmd(zoom,0.675;halign,0);
		OnCommand= function(self)
			self:shadowlength(1):skewx(-0.125):x(label_text:GetZoomedWidth()+8)
		end,
	};
};
