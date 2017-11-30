return Def.HelpDisplay {
	File = THEMEMAN:GetPathF("HelpDisplay", "text");
	InitCommand=function(self)
		local s = THEMEMAN:GetString(Var "LoadingScreen","HelpText");
		self:SetTipsColonSeparated(s);
	end;
	SetHelpTextCommand=function(self, params)
		self:SetTipsColonSeparated( params.Text );
	end;
};