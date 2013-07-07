return Def.Sprite {
	Texture=NOTESKIN:GetPath( '_down', 'explosion' );
	Frame0000=0;
	Delay0000=1;
	InitCommand=function(self)
		self:diffuseblink();
		self:effectcolor1(1, 1, 1, 0.8);
		self:effectcolor2(1, 1, 1, 1);
		self:effectclock('beat');
		self:effectperiod(0.25);
	end;
};