return Def.Sprite {
	Texture=NOTESKIN:GetPath( '_down', 'tap note' );
	Frames = Sprite.LinearFrames( 8, 1 );
	InitCommand=function(self)
		self:setstate(2);
	end;
	DrawTapNoteMessageCommand=function(self,parent)
		parent:spin();
	end;
};
