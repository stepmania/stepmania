local function MakeHelpDisplay()
	local X = ScreenMetric("HelpX");
	local Y = ScreenMetric("HelpY");
	local On = ScreenMetric("HelpOnCommand");
	local Off = ScreenMetric("HelpOffCommand");

	local t = Def.HelpDisplay {
		File = THEME:GetPathF("HelpDisplay", "text");

		InitCommand=function(self)
			local s = ScreenString("HelpText");
			self:SetTipsColonSeparated(s);
		end;

		SetHelpTextCommand=function(self, params)
			self:SetTipsColonSeparated( params.Text );
		end;
		OnCommand=function(self)
			self:x( X );
			self:y( Y );
			On(self);
		end;
		OffCommand=Off;
	};
	return t;
end

local t = Def.ActorFrame { };
t[#t+1] = MakeHelpDisplay();
return t;

