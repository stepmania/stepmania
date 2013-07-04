local t = Def.ActorFrame{
	LoadActor("EditMenu Left")..{
		BeginCommand=function(self)
			self:zoomx(-1);
		end;
	};
};

return t;