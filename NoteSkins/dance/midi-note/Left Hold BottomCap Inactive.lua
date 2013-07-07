return Def.Sprite {
	Texture=NOTESKIN:GetPath('Left','Hold BottomCap Active');
	InitCommand=function(self)
		self:diffuse(color("0.5,0.5,0.5,0.5"));
	end;
};