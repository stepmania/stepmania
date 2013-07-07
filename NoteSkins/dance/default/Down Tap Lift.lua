local t = Def.ActorFrame {
	Def.Sprite {
		Texture="_arrow";
		Frame0000=7;
		Delay0000=1;
		InitCommand=function(self)
			self:animate(false);
			self:pulse();
			self:effectclock("beat");
			self:effectmagnitude(0.9, 1, 1);
		end;
	};
};
return t;
