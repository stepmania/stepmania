-- I have to do it like this because I think the Set command in the metrics is screwing me over
local t = Def.ActorFrame{
	-- todo: make this less stupid
	Def.Sprite{
		InitCommand=cmd(x,8;horizalign,right),
		SetMessageCommand=function(self,param)
			local path = "Themes/"..THEME:GetCurThemeName().."/Graphics/_StepsType/" .. ToEnumShortString(param.StepsType) .. ".png"
			if FILEMAN:DoesFileExist(path) then
				self:Load( path )
			else
				self:Load( THEME:GetPathG("","_StepsType/missing") )
			end
		end
	}
}

return t;