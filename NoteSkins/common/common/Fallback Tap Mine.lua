-- commands don't behave if this isn't wrapped in a frame
return Def.ActorFrame {
	Def.Sprite {
		Texture = NOTESKIN:GetPath('_mine', ''),
		InitCommand = NOTESKIN:GetMetricA("Mine", "InitCommand")
	}
}
