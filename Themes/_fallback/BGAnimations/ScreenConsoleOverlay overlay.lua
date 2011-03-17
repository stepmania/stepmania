local t = Def.ActorFrame {
	Def.ActorFrame {
		InitCommand=cmd(Center);
	--[[ 	ToggleConsoleDisplayMessageCommand=function(self)
			bVisible = 1 - bVisible;
			bShow = (bVisible >= 1) and true or false;
			self:visible(bShow);
		end; --]]
		Def.Quad {
			InitCommand(zoomto,64,64;spin;);
			ToggleConsoleDisplayMessageCommand=cmd(zoomto,345,345;visible,true);
		};
	};
};
return t