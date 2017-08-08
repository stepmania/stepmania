return Def.HelpDisplay {
	File = THEME:GetPathF("HelpDisplay", "text");
	InitCommand=function(self)
		local s = THEME:GetString(Var "LoadingScreen","HelpText");
		self:SetTipsColonSeparated(s);
	end;
	SetHelpTextCommand=function(self, params)
		self:SetTipsColonSeparated( params.Text );
	end;
};