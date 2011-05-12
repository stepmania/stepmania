-- Loads up a slew of objects to load into the screen, like how 3.9 does.
-- I prefer to keep these optional, incase another screen wants to hide 
-- these elements.
local t = Def.ActorFrame {};
-- Image displayed on the top of the screen
t[#t+1] = StandardDecorationFromFileOptional("Header","Header");
-- Text displayed at the top of the screen
t[#t+1] = StandardDecorationFromFileOptional("TextHeader","TextHeader") .. {
	BeginCommand=cmd(playcommand,"Set");
	SetCommand=function(self)
		local sText = '_';
		if SCREENMAN:GetTopScreen() then
			if SCREENMAN:GetTopScreen():GetName() ~= nil then
				sText = THEME:GetString(SCREENMAN:GetTopScreen():GetName(), "HeaderText");
			else
				sText = 'No Screen Name';
			end
		else
			sText = 'No Top Screen';
		end
		--
		self:settext(sText);
	end;
};
-- Image displayed on the bottom of the screen
t[#t+1] = StandardDecorationFromFileOptional("Footer","Footer");
t[#t+1] = StandardDecorationFromFileOptional( "HelpDisplay", "HelpDisplay" ) .. {
	InitCommand=function(self)
		local s = THEME:GetString(Var "LoadingScreen","HelpText");
		self:SetTipsColonSeparated(s);
	end;
	SetHelpTextCommand=function(self, params)
		self:SetTipsColonSeparated( params.Text );
	end;
};
return t