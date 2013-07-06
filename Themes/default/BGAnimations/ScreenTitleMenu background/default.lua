local t = Def.ActorFrame {};
t[#t+1] = Def.ActorFrame {
	FOV=90;
	InitCommand=function(self)
		self:Center();
	end;
	Def.Quad {
		InitCommand=function(self)
			self:scaletoclipped(SCREEN_WIDTH,SCREEN_HEIGHT);
		end;
		OnCommand=function(self)
			self:diffuse(color("#FFCB05"));
			self:diffusebottomedge(color("#F0BA00"));
		end;
	};
	Def.ActorFrame {
		Def.ActorFrame {
			InitCommand=function(self)
				self:hide_if(hideFancyElements);
			end;
			OffCommand=function(self)
				self:decelerate(0.25);
				self:rotationx(-90 / 4 * 3.5);
				self:y(SCREEN_CENTER_Y);
			end;
			ShiftCommand=function(self)
				self:smooth(1.25);
				self:z(256);
				self:sleep(2);
				self:smooth(1.25);
				self:z(0);
				self:sleep(2);
				self:queuecommand("Shift");
			end;
			FlipCommand=function(self)
				self:smooth(0.5);
				self:rotationy(180);
				self:sleep(2);
				self:smooth(0.5);
				self:rotationy(360);
				self:sleep(1);
				self:rotationy(0);
				self:sleep(1);
				self:queuecommand("Flip");
			end;
			LoadActor(THEME:GetPathB("ScreenWithMenuElements","background/_checkerboard")) .. {
				InitCommand=function(self)
					self:zoomto(SCREEN_WIDTH * 2, SCREEN_HEIGHT * 2);
					self:customtexturerect(0, 0, SCREEN_WIDTH * 4 / 256, SCREEN_HEIGHT * 4 / 256);
				end;
				OnCommand=function(self)
					self:texcoordvelocity(0, 0.5);
					self:diffuse(color("#ffd400"));
					self:diffusealpha(0.5);
					self:fadetop(1);
					self:fadebottom(1);
				end;
			};
		};
	};
};
return t;