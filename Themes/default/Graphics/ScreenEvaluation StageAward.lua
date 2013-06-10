return LoadFont( "_venacti bold 15" ) .. {
	InitCommand=cmd(strokecolor,color("#e6ffff");diffusetopedge,color("#007fa8");diffusebottomedge,color("#2fbbdc");shadowlength,0;maxwidth,144;glowshift;);
	SetCommand=function(self,param)
		if param.StageAward then 
			self:settext( StageAwardToLocalizedString(param.StageAward) );
		else
			self:visible(false);
		end
	end
};