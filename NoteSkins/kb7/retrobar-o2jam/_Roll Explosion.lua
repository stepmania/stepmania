return LoadActor("_bar hold explosion bright")..{
	CheckpointHitCommand=function(self)
		self:diffusealpha(0);
	end;
};