local t = Def.ActorFrame {
	Def.Sprite {
		Texture=NOTESKIN:GetPath( '_down', 'tap mine' );
		Frame0000=0;
		Delay0000=1;
		InitCommand=function(self)
			self:spin();
			self:effectclock('beat');
			self:effectmagnitude(0, 0, -180);
		end;
	};
};
return t;
