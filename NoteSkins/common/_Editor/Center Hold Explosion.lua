return Def.Sprite {
	Texture=NOTESKIN:GetPath( '_center', 'explosion' );
	InitCommand=function(self)
		self:blend("BlendMode_Add");
		self:finishtweening();
		self:diffusealpha(0.2);
		self:zoom(0.6);
		self:linear(0.1);
		self:diffusealpha(0);
		self:zoom(0.8);
	end;
};