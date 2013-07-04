return Def.ActorFrame {
	Def.DeviceList {
		Font="Common normal";
		InitCommand=function(self)
			self:x(SCREEN_LEFT + 20);
			self:y(SCREEN_TOP + 80);
			self:zoom(0.8);
			self:halign(0);
		end;
	};

	Def.InputList {
		Font="Common normal";
		InitCommand=function(self)
			self:x(SCREEN_CENTER_X - 250);
			self:y(SCREEN_CENTER_Y);
			self:zoom(1);
			self:halign(0);
			self:vertspacing(8);
		end;
	};
};
