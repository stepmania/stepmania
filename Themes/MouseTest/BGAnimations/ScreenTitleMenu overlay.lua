local t = Def.ActorFrame{
	LoadActor(THEME:GetPathG("_click","target"))..{
		InitCommand=function(self)
			self:diffusealpha(0);
			self:Real();
		end;
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
		InitCommand=function(self)
			self:shadowlength(1);
			self:zoom(0.5);
			self:align(0, 1);
		end;
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