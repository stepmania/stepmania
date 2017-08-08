local sString;
local t = Def.ActorFrame{
	
	LoadFont("Common normal")..{
		InitCommand=cmd(shadowlength,1);
		SetMessageCommand=function(self,param)
			if param.StepsType then
				sString = THEME:GetString("StepsDisplay StepsType",ToEnumShortString(param.StepsType));
				if param.Steps and param.Steps:IsAutogen() then
					self:diffusebottomedge(color("0.75,0.75,0.75,1"));
				else
					self:diffuse(Color("White"));
				end;
				self:settext( sString );
			end;
		end;
	};
	-- argh this isn't working as nicely as I would've hoped...
	--[[
	Def.Sprite{
		SetMessageCommand=function(self,param)
			self:Load( THEME:GetPathG("","_StepsType/"..ToEnumShortString(param.StepsType)) );
			self:diffusealpha(0.5);
		end;
	};
	--]]
};

return t;