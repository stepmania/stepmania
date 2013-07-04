local sString;
local t = Def.ActorFrame{
	LoadFont("Common normal")..{
		InitCommand=function(self)
			self:shadowlength(1);
			self:horizalign(left);
			self:zoom(0.45);
			self:skewx(-0.125);
		end;
		SetMessageCommand=function(self,param)
			sString = THEME:GetString("StepsListDisplayRow StepsType",ToEnumShortString(param.StepsType));
			if param.Steps and param.Steps:IsAutogen() then
				self:diffusebottomedge(color("0.75,0.75,0.75,1"));
			else
				self:diffuse(Color("White"));
			end;
			self:settext( sString );
		end;
	};
};

return t;