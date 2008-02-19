return Def.ActorFrame {
	SetCommand=function(self,param)
			local Steps = param.Steps;
			local StepsType;
			if Steps then
				StepsType = Steps:GetStepsType();
			end
			--Trace( StepsType );
			self:GetChild("IconSingle"):visible( StepsType == "StepsType_Pump_Single" );
			self:GetChild("IconDouble"):visible( StepsType == "StepsType_Pump_Double" );
		end;
	LoadFont( "_venacti bold 26" ) .. {
		Text="XXXX";
		InitCommand=cmd(x,-110;y,-1;zoom,.5;shadowlengthx,0;shadowlengthy,2;shadowcolor,color("#00000077"););
		SetCommand=function(self,param)
				local s = DifficultyAndStepsTypeToLocalizedString( param.Difficulty, param.StepsType );
				if param.Difficulty == "Difficulty_Edit" then
					s = param.EditDescription;
				end
				s = string.upper(s);	-- TODO: string.upper doesn't work correctly for French
				self:settext( s );
				local c = DifficultyAndStepsTypeToColor( param.Difficulty, param.StepsType );
				self:diffuse( c );
			end;
	};
	LoadActor( THEME:GetPathG("","_StepsType icon single") ) .. {
		Name="IconSingle";
		InitCommand=cmd(x,100;y,2;;visible,false);
	};
	LoadActor( THEME:GetPathG("","_StepsType icon double") ) .. {
		Name="IconDouble";
		InitCommand=cmd(x,100;y,2;visible,false);
	};
};
