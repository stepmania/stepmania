return Def.ActorFrame {
	Def.ActorFrame {
		OnCommand=function(self)
			self:x(SCREEN_CENTER_X-20);
		end;

		-- Initial glow around receptors
		LoadActor("tapglow") .. {
			OnCommand=function(self)
				self:x(85);
				self:y(95);
				self:zoom(0.7);
				self:rotationz(90);
				self:diffuseshift();
				self:effectcolor1(1, 0.93333, 0.266666, 0.4);
				self:effectcolor2(1, 1, 1, 1);
				self:effectperiod(0.25);
				self:effectmagnitude(0, 1, 0);
				self:diffusealpha(0);
				self:sleep(6);
				self:linear(0);
				self:diffusealpha(1);
				self:sleep(1.7);
				self:linear(0);
				self:diffusealpha(0);
			end;
		};
		LoadActor("tapglow") .. {
			OnCommand=function(self)
				self:x(275);
				self:y(95);
				self:zoom(0.7);
				self:rotationz(270);
				self:diffuseshift();
				self:effectcolor1(1, 0.93333, 0.266666, 0.4);
				self:effectcolor2(1, 1, 1, 1);
				self:effectperiod(0.25);
				self:effectmagnitude(0, 1, 0);
				self:diffusealpha(0);
				self:sleep(6);
				self:linear(0);
				self:diffusealpha(1);
				self:sleep(1.7);
				self:linear(0);
				self:diffusealpha(0);
			end;
		};
		LoadActor("tapglow") .. {
			OnCommand=function(self)
				self:x(212);
				self:y(95);
				self:zoom(0.7);
				self:rotationz(180);
				self:diffuseshift();
				self:effectcolor1(1, 0.93333, 0.266666, 0.4);
				self:effectcolor2(1, 1, 1, 1);
				self:effectperiod(0.25);
				self:effectmagnitude(0, 1, 0);
				self:diffusealpha(0);
				self:sleep(6);
				self:linear(0);
				self:diffusealpha(1);
				self:sleep(1.7);
				self:linear(0);
				self:diffusealpha(0);
			end;
		};
		LoadActor("tapglow") .. {
			OnCommand=function(self)
				self:x(148);
				self:y(95);
				self:zoom(0.7);
				self:diffuseshift();
				self:effectcolor1(1, 0.93333, 0.266666, 0.4);
				self:effectcolor2(1, 1, 1, 1);
				self:effectperiod(0.25);
				self:effectmagnitude(0, 1, 0);
				self:diffusealpha(0);
				self:sleep(6);
				self:linear(0);
				self:diffusealpha(1);
				self:sleep(1.7);
				self:linear(0);
				self:diffusealpha(0);
			end;
		};

		LoadActor("tapglow") .. {
			OnCommand=function(self)
				self:x(148);
				self:y(95);
				self:zoom(0.7);
				self:diffuseshift();
				self:effectcolor1(1, 0.93333, 0.266666, 0.4);
				self:effectcolor2(1, 1, 1, 1);
				self:effectperiod(0.25);
				self:effectmagnitude(0, 1, 0);
				self:diffusealpha(0);
				self:sleep(9.7);
				self:linear(0);
				self:diffusealpha(1);
				self:sleep(1.7);
				self:linear(0);
				self:diffusealpha(0);
			end;
		};

		-- 2nd step UP
		LoadActor("tapglow") .. {
			OnCommand=function(self)
				self:x(212);
				self:y(95);
				self:zoom(0.7);
				self:rotationz(180);
				self:diffuseshift();
				self:effectcolor1(1, 0.93333, 0.266666, 0.4);
				self:effectcolor2(1, 1, 1, 1);
				self:effectperiod(0.25);
				self:effectmagnitude(0, 1, 0);
				self:diffusealpha(0);
				self:sleep(12.7);
				self:linear(0);
				self:diffusealpha(1);
				self:sleep(1.7);
				self:linear(0);
				self:diffusealpha(0);
			end;
		};

		-- 3rd step UP
		LoadActor("tapglow") .. {
			OnCommand=function(self)
				self:x(84);
				self:y(95);
				self:zoom(0.7);
				self:rotationz(90);
				self:diffuseshift();
				self:effectcolor1(1, 0.93333, 0.266666, 0.4);
				self:effectcolor2(1, 1, 1, 1);
				self:effectperiod(0.25);
				self:effectmagnitude(0, 1, 0);
				self:diffusealpha(0);
				self:sleep(15.7);
				self:linear(0);
				self:diffusealpha(1);
				self:sleep(1.7);
				self:linear(0);
				self:diffusealpha(0);
			end;
		};

		-- 4th step jump
		LoadActor("tapglow") .. {
			OnCommand=function(self)
				self:x(85);
				self:y(95);
				self:zoom(0.7);
				self:rotationz(90);
				self:diffuseshift();
				self:effectcolor1(1, 0.93333, 0.266666, 0.4);
				self:effectcolor2(1, 1, 1, 1);
				self:effectperiod(0.25);
				self:effectmagnitude(0, 1, 0);
				self:diffusealpha(0);
				self:sleep(18.7);
				self:linear(0);
				self:diffusealpha(1);
				self:sleep(1.7);
				self:linear(0);
				self:diffusealpha(0);
			end;
		};
		LoadActor("tapglow") .. {
			OnCommand=function(self)
				self:x(275);
				self:y(95);
				self:zoom(0.7);
				self:rotationz(270);
				self:diffuseshift();
				self:effectcolor1(1, 0.93333, 0.266666, 0.4);
				self:effectcolor2(1, 1, 1, 1);
				self:effectperiod(0.25);
				self:effectmagnitude(0, 1, 0);
				self:diffusealpha(0);
				self:sleep(18.7);
				self:linear(0);
				self:diffusealpha(1);
				self:sleep(1.7);
				self:linear(0);
				self:diffusealpha(0);
			end;
		};

		-- miss step
		LoadActor("healthhilight") .. {
			OnCommand=function(self)
				self:x(180);
				self:y(40);
				self:zoom(0.7);
				self:diffuseshift();
				self:effectcolor1(1, 0.93333, 0.266666, 0.4);
				self:effectcolor2(1, 1, 1, 1);
				self:effectperiod(0.25);
				self:effectmagnitude(0, 1, 0);
				self:diffusealpha(0);
				self:sleep(22.7);
				self:linear(0);
				self:diffusealpha(1);
				self:sleep(1.7);
				self:linear(0);
				self:diffusealpha(0);
			end;
		};
	};

	-- messages
	LoadFont("Common Normal") .. {
		Text=ScreenString("How To Play StepMania"),
		InitCommand=function(self)
			self:zbuffer(1);
			self:z(20);
			self:x(SCREEN_CENTER_X);
			self:y(SCREEN_CENTER_Y);
			self:shadowlength(1);
			self:strokecolor(Color("Outline"));
		end;
		OnCommand=function(self)
			self:diffusealpha(0);
			self:zoom(4);
			self:sleep(0.0);
			self:linear(0.3);
			self:diffusealpha(1);
			self:zoom(1);
			self:sleep(1.8);
			self:linear(0.3);
			self:zoom(0.75);
			self:x(170);
			self:y(60);
		end;
	};
	LoadActor("feet") .. {
		OnCommand=function(self)
			self:z(20);
			self:x(SCREEN_CENTER_X);
			self:y(SCREEN_CENTER_Y);
			self:addx(-SCREEN_WIDTH);
			self:sleep(2.4);
			self:decelerate(0.3);
			self:addx(SCREEN_WIDTH);
			self:sleep(2);
			self:linear(0.3);
			self:zoomy(0);
		end;
	};

	LoadActor("_message tap")..{
		OnCommand=function(self)
			self:sleep(6);
			self:queuecommand("Show");
		end;
	};
	LoadActor("_message tap")..{
		OnCommand=function(self)
			self:sleep(9.7);
			self:queuecommand("Show");
		end;
	};
	LoadActor("_message tap")..{
		OnCommand=function(self)
			self:sleep(12.7);
			self:queuecommand("Show");
		end;
	};
	LoadActor("_message tap")..{
		OnCommand=function(self)
			self:sleep(15.7);
			self:queuecommand("Show");
		end;
	};
	LoadActor("_message jump")..{
		OnCommand=function(self)
			self:sleep(18.7);
			self:queuecommand("Show");
		end;
	};
	LoadActor("_message miss")..{
		OnCommand=function(self)
			self:sleep(22.7);
			self:queuecommand("Show");
		end;
	};
};
