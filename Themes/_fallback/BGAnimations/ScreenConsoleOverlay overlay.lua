local t = Def.ActorFrame {
	Def.ActorFrame {
		InitCommand=function(self)
			self:Center();
		end;
--[[ 	
		ToggleConsoleDisplayMessageCommand=function(self)
			bVisible = 1 - bVisible;
			bShow = (bVisible >= 1) and true or false;
			self:visible(bShow);
		end;
--]]
		Def.Quad {
			InitCommand(zoomto,64,64;spin;);
			ToggleConsoleDisplayMessageCommand=function(self)
				self:zoomto(345, 345);
				self:visible(true);
			end;
		};
	};
};
return t