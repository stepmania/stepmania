local children = { };

children[#children+1] = Def.HelpDisplay {
	File = THEME:GetPathF("HelpDisplay", "text");

	InitCommand=function(self)
		local s = ScreenString("HelpText");
		self:SetTipsColonSeparated(s);
	end;

	SetHelpTextCommand=function(self, params)
		self:SetTipsColonSeparated( params.Text );
	end;
	OnCommand=function(self)
		self:x( ScreenMetric("HelpX") );
		self:y( ScreenMetric("HelpY") );
		local f = ScreenMetric("HelpOnCommand");
		f(self);
	end;
	OffCommand=function(self)
		local f = ScreenMetric("HelpOffCommand");
		f(self);
	end;
};

return Def.ActorFrame {
	children = children;
};

