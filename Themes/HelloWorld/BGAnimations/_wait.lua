local time = ...
if not time then time = 1.0 end

return Def.Actor{ 
	OnCommand=function(self)
		self:sleep(time);
	end;
};