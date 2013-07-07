local t = Def.ActorFrame {
	Def.Sprite {
		Texture=NOTESKIN:GetPath( '_down', 'tap mine underlay' );
		Frames = Sprite.LinearFrames( 1, 1 );
		InitCommand=function(self)
			self:diffuseshift();
			self:effectcolor1(0.4, 0, 0, 1);
			self:effectcolor2(1, 0, 0, 1);
			self:effectclock('beat');
		end;
	};
		Def.Sprite {
		Texture=NOTESKIN:GetPath( '_down', 'tap mine base' );
		Frames = Sprite.LinearFrames( 1, 1 );
		InitCommand=function(self)
			self:spin();
			self:effectclock('beat');
			self:effectmagnitude(0, 0, 80);
		end;
	};
		Def.Sprite {
		Texture=NOTESKIN:GetPath( '_down', 'tap mine overlay' );
		Frames = Sprite.LinearFrames( 1, 1 );
		InitCommand=function(self)
			self:spin();
			self:effectclock('beat');
			self:effectmagnitude(0, 0, -40);
		end;
	};
};
return t;
