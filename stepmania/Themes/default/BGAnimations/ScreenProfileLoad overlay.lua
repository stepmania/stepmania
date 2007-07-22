return Def.ActorFrame {
	BeginCommand=function(self)
		self:queuecommand( "Load" )
	end;
	LoadCommand=function()
		SCREENMAN:GetTopScreen():Continue()
	end;
}
