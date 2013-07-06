local raveChildren

local bg = Def.ActorFrame{
	Def.Quad{
		InitCommand=function(self)
			self:FullScreen();
			self:diffuse(color("0,0,0,0"));
		end;
		OnCommand=function(self)
			self:linear(5);
			self:diffusealpha(1);
		end;
	};

	Def.ActorFrame{
		Name="RaveMessages";
		InitCommand=function(self)
			raveChildren = self:GetChildren()
			self:visible(GAMESTATE:GetPlayMode() == 'PlayMode_Rave')

			raveChildren.P1Win:visible(false)
			raveChildren.P2Win:visible(false)
			raveChildren.Draw:visible(false)
		end;
		OffCommand=function(self)
			local p1Win = GAMESTATE:IsWinner(PLAYER_1)
			local p2Win = GAMESTATE:IsWinner(PLAYER_2)

			if GAMESTATE:IsWinner(PLAYER_1) then
				raveChildren.P1Win:visible(true)
			elseif GAMESTATE:IsWinner(PLAYER_2) then
				raveChildren.P2Win:visible(true)
			else
				raveChildren.Draw:visible(true)
			end
		end;

		LoadActor(THEME:GetPathG("_rave result","P1"))..{
			Name="P1Win";
			InitCommand=function(self)
				self:Center();
				self:cropbottom(1);
				self:fadebottom(1);
			end;
			OnCommand=function(self)
				self:sleep(2);
				self:linear(0.5);
				self:cropbottom(0);
				self:fadebottom(0);
				self:sleep(1.75);
				self:linear(0.25);
				self:diffusealpha(0);
			end;
		};
		LoadActor(THEME:GetPathG("_rave result","P2"))..{
			Name="P2Win";
			InitCommand=function(self)
				self:Center();
				self:cropbottom(1);
				self:fadebottom(1);
			end;
			OnCommand=function(self)
				self:sleep(2);
				self:linear(0.5);
				self:cropbottom(0);
				self:fadebottom(0);
				self:sleep(1.75);
				self:linear(0.25);
				self:diffusealpha(0);
			end;
		};
		LoadActor(THEME:GetPathG("_rave result","draw"))..{
			Name="Draw";
			InitCommand=function(self)
				self:Center();
				self:cropbottom(1);
				self:fadebottom(1);
			end;
			OnCommand=function(self)
				self:sleep(2);
				self:linear(0.5);
				self:cropbottom(0);
				self:fadebottom(0);
				self:sleep(1.75);
				self:linear(0.25);
				self:diffusealpha(0);
			end;
		};
	};
};

return bg