return Def.HelpDisplay {
	File = THEME:GetPathF("HelpDisplay", "text");
	InitCommand=function(self)
		local s = THEME:GetString(Var "LoadingScreen","AlternateHelpText");
		self:SetTipsColonSeparated(s);
	end;
	SetHelpTextCommand=function(self, params)
		self:SetTipsColonSeparated( params.Text );
	end;
};
--[[ local sString = THEME:GetString(Var "LoadingScreen","AlternateHelpText");
local tItems = split(sString,"&");

local t = Def.ActorScroller {
	NumItemsToDraw=#tItems;
	SecondsPerItem=1.25;
	TransformFunction=function( self, offset, itemIndex, numItems )
		self:x( offset*74 );
	end;
	InitCommand=cmd(SetLoop,true);
-- 	OnCommand=cmd(scrollwithpadding,10,0);
};

for i=1,#tItems do
	t[#t+1] = Def.ActorFrame {
		LoadFont("HelpDisplay", "text") .. {
			Text=tostring(tItems[i]);
			OnCommand=THEME:GetMetric( Var "LoadingScreen","HelpOnCommand");
		};
	};
end

return t; --]]
