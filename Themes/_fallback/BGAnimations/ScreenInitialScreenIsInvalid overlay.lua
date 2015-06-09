return Def.ActorFrame{
	Def.BitmapText{
		Font= "Common Normal",
		Text= THEME:GetString("ScreenInitialScreenIsInvalid", "InvalidScreenExplanation"),
		InitCommand= function(self)
			self:xy(_screen.cx, _screen.cy):wrapwidthpixels(_screen.w-16)
				:diffuse{1, 1, 1, 1}:strokecolor{0, 0, 0, 1}
		end
	}
}
