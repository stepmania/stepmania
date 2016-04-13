local sString;

local stGraphics = {
	-- dance (ok)
	StepsType_Dance_Single = "_dance-single",
	StepsType_Dance_Double = "_dance-double",
	StepsType_Dance_Couple = "_dance-couple",
	StepsType_Dance_Solo = "_solo",
	StepsType_Dance_Routine = "_dance-routine",
	StepsType_Dance_Threepanel = "_dance-3panel",
	-- pump (ok)
	StepsType_Pump_Single = "_pump-single",
	StepsType_Pump_Halfdouble = "_pump-halfdouble",
	StepsType_Pump_Double = "_pump-double",
	StepsType_Pump_Couple = "_pump-couple",
	StepsType_Pump_Routine = "_pump-routine",
	-- kb7
	StepsType_Kb7_Single = "_kb7",
	-- ez2
	StepsType_Ez2_Single = "_base",
	StepsType_Ez2_Double = "_base",
	StepsType_Ez2_Real = "_base",
	-- para
	StepsType_Para_Single = "_para",
	-- ds3ddx
	StepsType_Ds3ddx_Single = "_base",
	-- beat (ok)
	StepsType_Bm_Single5 = "_beat5",
	StepsType_Bm_Double5 = "_beat10",
	StepsType_Bm_Single7 = "_beat7",
	StepsType_Bm_Double7 = "_beat14",
	-- maniax
	StepsType_Maniax_Single = "_maniax-single",
	StepsType_Maniax_Double = "_maniax-double",
	-- techno (ok)
	StepsType_Techno_Single4 = "_dance-single",
	StepsType_Techno_Single5 = "_pump-single",
	StepsType_Techno_Single8 = "_techno8",
	StepsType_Techno_Double4 = "_dance-double",
	StepsType_Techno_Double5 = "_pump-double",
	-- popn
	StepsType_Pnm_Five = "_popn5",
	StepsType_Pnm_Nine = "_popn9",
	-- gametypes that don't matter
	StepsType_Lights_Cabinet = "_lights", -- nobody should play this mode (except to test lightmaps)
};

local t = Def.ActorFrame{
	Def.Sprite{
		SetMessageCommand=function(self,param)
			if param.StepsType then
				self:Load( THEME:GetPathG("","_stepstype/"..stGraphics[param.StepsType]) );
				-- broken as shit now
				--[[
				if param.Steps:IsAutogen() then
					self:rainbow();
				else
					self:stopeffect();
				end;
				--]]
			end;
		end;
	};
};

return t;