local t = Def.ActorFrame{
	LoadActor(THEME:GetPathG("_click","target"))..{
		InitCommand=cmd(diffusealpha,0;Real);
		LeftClickMessageCommand=function(self)
			MESSAGEMAN:Broadcast("MouseInput", { Input = "Left" });
		end;
		RightClickMessageCommand=function(self)
			MESSAGEMAN:Broadcast("MouseInput", { Input = "Right" });
		end;
		MiddleClickMessageCommand=function(self)
			MESSAGEMAN:Broadcast("MouseInput", { Input = "Middle" });
		end;
		MouseWheelUpMessageCommand=function(self)
			MESSAGEMAN:Broadcast("MouseInput", { Input = "WheelUp" });
		end;
		MouseWheelDownMessageCommand=function(self)
			MESSAGEMAN:Broadcast("MouseInput", { Input = "WheelDown" });
		end;
		MouseInputMessageCommand=function(self,param)
			local cX = INPUTFILTER:GetMouseX()
			local cY = INPUTFILTER:GetMouseY()
			if param.Input == "Left" then
				self:stoptweening()
				self:x(cX)
				self:y(cY)
				self:diffusealpha(1)
				self:sleep(1)
				self:diffusealpha(0)
			end
		end
	};
	LoadFont("common normal")..{
		Name="Coords";
		InitCommand=cmd(shadowlength,1;zoom,0.5;align,0,1);
		MouseInputMessageCommand=function(self,param)
			local cX = INPUTFILTER:GetMouseX()
			local cY = INPUTFILTER:GetMouseY()
			if param.Input == "Left" then
				self:stoptweening()
				self:x(cX + 8)
				self:y(cY + 16)
				self:settext("(".. string.format("%i",cX) ..",".. string.format("%i",cY) ..")")
				self:diffusealpha(1)
				self:sleep(1)
				self:diffusealpha(0)
			end
		end
	};
};

return t;