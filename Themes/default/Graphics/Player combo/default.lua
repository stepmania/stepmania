local c;
local cf;
local canAnimate = false;
local player = Var "Player";
local ShowComboAt = THEME:GetMetric("Combo", "ShowComboAt");
local Pulse = THEME:GetMetric("Combo", "PulseCommand");
local PulseLabel = THEME:GetMetric("Combo", "PulseLabelCommand");

local NumberMinZoom = THEME:GetMetric("Combo", "NumberMinZoom");
local NumberMaxZoom = THEME:GetMetric("Combo", "NumberMaxZoom");
local NumberMaxZoomAt = THEME:GetMetric("Combo", "NumberMaxZoomAt");

local LabelMinZoom = THEME:GetMetric("Combo", "LabelMinZoom");
local LabelMaxZoom = THEME:GetMetric("Combo", "LabelMaxZoom");

local ShowFlashyCombo = player_config:get_data(player).FlashyCombo

local t = Def.ActorFrame {
	InitCommand= function(self)
		if player_config:get_data(player).ComboUnderField then
			self:draworder(notefield_draw_order.under_field)
		else
			self:draworder(notefield_draw_order.over_field)
		end
		c = self:GetChildren();
		cf = c.ComboFrame:GetChildren();
		cf.Number:visible(false);
		cf.ComboLabel:visible(false)
		cf.MissLabel:visible(false)
	end,
	-- flashy combo elements:
	LoadActor(THEME:GetPathG("Combo","100Milestone"), player) .. {
		Name="OneHundredMilestone";
		InitCommand=cmd(visible,ShowFlashyCombo);
		FiftyMilestoneCommand=cmd(playcommand,"Milestone");
	};
	LoadActor(THEME:GetPathG("Combo","1000Milestone")) .. {
		Name="OneThousandMilestone";
		InitCommand=cmd(visible,ShowFlashyCombo);
		ToastyAchievedMessageCommand=cmd(playcommand,"Milestone");
	};
	-- normal combo elements:
	Def.ActorFrame {
		Name="ComboFrame";
		LoadFont( "Combo", "numbers" ) .. {
			Name="Number";
			OnCommand = THEME:GetMetric("Combo", "NumberOnCommand");
		};
		LoadFont("_roboto condensed Bold italic 24px") .. {
			Name="ComboLabel";
			InitCommand=function(self)
				self:settext(string.upper(THEME:GetString("ScreenGameplay","Combo")));
			end;
			OnCommand = THEME:GetMetric("Combo", "LabelOnCommand");
		};
		LoadFont("_roboto condensed Bold italic 24px") .. {
			Name="MissLabel";
			InitCommand=function(self)
				self:settext(string.upper(THEME:GetString("ScreenGameplay","Misses")));
			end;
			OnCommand = THEME:GetMetric("Combo", "MissLabelOnCommand");
		};
	};
	-- Milestones:
	-- 25,50,100,250,600 Multiples;
--[[ 		if (iCombo % 100) == 0 then
			c.OneHundredMilestone:playcommand("Milestone");
		elseif (iCombo % 250) == 0 then
			-- It should really be 1000 but thats slightly unattainable, since
			-- combo doesnt save over now.
			c.OneThousandMilestone:playcommand("Milestone");
		else
			return
		end; --]]
 	TwentyFiveMilestoneCommand=function(self,parent)
		if ShowFlashyCombo then
			(cmd(finishtweening;addy,-4;bounceend,0.125;addy,4))(self);
		end;
	end;
	--]]
	--[[
	ToastyAchievedMessageCommand=function(self,params)
		if params.PlayerNumber == player then
			(cmd(thump,2;effectclock,'beat'))(c.ComboFrame);
		end;
	end;
	ToastyDroppedMessageCommand=function(self,params)
		if params.PlayerNumber == player then
			(cmd(stopeffect))(c.ComboFrame);
		end;
	end; --]]
	ComboCommand=function(self, param)
		local iCombo = param.Misses or param.Combo;
		if not iCombo or iCombo < ShowComboAt then
			cf.Number:visible(false);
			cf.ComboLabel:visible(false)
			cf.MissLabel:visible(false)
			return;
		end

		cf.ComboLabel:visible(false)
		cf.MissLabel:visible(false)

		param.Zoom = scale( iCombo, 0, NumberMaxZoomAt, NumberMinZoom, NumberMaxZoom );
		param.Zoom = clamp( param.Zoom, NumberMinZoom, NumberMaxZoom );

		param.LabelZoom = scale( iCombo, 0, NumberMaxZoomAt, LabelMinZoom, LabelMaxZoom );
		param.LabelZoom = clamp( param.LabelZoom, LabelMinZoom, LabelMaxZoom );

		if param.Combo then
			cf.ComboLabel:visible(true)
			cf.MissLabel:visible(false)
		else
			cf.ComboLabel:visible(false)
			cf.MissLabel:visible(true)
		end

		cf.Number:visible(true);
		cf.Number:settext( string.format("%i", iCombo) );
		-- FullCombo Rewards
		if param.FullComboW1 then
			cf.Number:diffuse(color("#00aeef"));
			cf.ComboLabel:diffuse(color("#C7E5F0")):diffusebottomedge(color("#00aeef")):strokecolor(color("#0E3D53"));
		elseif param.FullComboW2 then
			cf.Number:diffuse(color("#F3D58D"));
			cf.ComboLabel:diffuse(color("#FAFAFA")):diffusebottomedge(color("#F3D58D")):strokecolor(color("#53450E"));
		elseif param.FullComboW3 then
			cf.Number:diffuse(color("#94D658"));
			cf.ComboLabel:diffuse(color("#CFE5BC")):diffusebottomedge(color("#94D658")):strokecolor(color("#12530E"));
		elseif param.Combo then
			-- Player 1's color is Red, which conflicts with the miss combo.
			-- instead, just diffuse to white for now. -aj
			--c.Number:diffuse(PlayerColor(player));
			cf.Number:diffuse(Color("White"));
			cf.Number:strokecolor(Color("Stealth"));
			cf.Number:stopeffect();
			cf.ComboLabel:diffuse(color("#F5CB92")):diffusebottomedge(color("#EFA97A")):strokecolor(color("#602C1B"));
		else
			cf.Number:diffuse(color("#ff0000"));
			cf.MissLabel:diffuse(color("#ff0000"));
			cf.ComboLabel:diffuse(color("#ff0000"));
			cf.MissLabel:strokecolor(color("#000000"));
			cf.ComboLabel:strokecolor(color("#000000"));
			cf.Number:stopeffect();
		end
		-- Pulse
		Pulse( cf.Number, param );
		if param.Combo then
			PulseLabel( cf.ComboLabel, param );
		else
			PulseLabel( cf.MissLabel, param );
		end
		-- Milestone Logic
	end;
--[[ 	ScoreChangedMessageCommand=function(self,param)
		local iToastyCombo = param.ToastyCombo;
		if iToastyCombo and (iToastyCombo > 0) then
-- 			(cmd(thump;effectmagnitude,1,1.2,1;effectclock,'beat'))(c.Number)
-- 			(cmd(thump;effectmagnitude,1,1.2,1;effectclock,'beat'))(c.Number)
		else
-- 			c.Number:stopeffect();
		end;
	end; --]]
};

return t;
