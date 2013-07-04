return Def.ActorFrame {
	LoadFont("Common Normal") .. {
		Text="BPM";
		InitCommand=function(self)
			self:horizalign(right);
			self:zoom(0.75);
			self:strokecolor(Color("Outline"));
		end;
	};
};