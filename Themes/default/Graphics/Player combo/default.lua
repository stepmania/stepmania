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

--different language support
local lang = THEME:GetCurLanguage()
local cur_dir= "/Themes/smtheme-fiftyone/Graphics/Player combo/"
local combo_label = cur_dir.."_combo"
local miss_label = cur_dir.."_misses"

if lang ~= "en" and FILEMAN:DoesFileExist(combo_label.." (lang "..lang..").png") and FILEMAN:DoesFileExist(miss_label.." (lang "..lang..").png")  then
	combo_label = combo_label.." (lang "..lang..").png"
	miss_label = miss_label.." (lang "..lang..").png"
else
	combo_label = combo_label..".png"
	miss_label = miss_label..".png"
end

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
		LoadActor(combo_label)..{
			Name="ComboLabel";
			OnCommand = THEME:GetMetric("Combo", "ComboLabelOnCommand");
		};
		LoadActor(miss_label)..{
			Name="MissLabel";
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
			cf.Number:diffuse( GameColor.Judgment["JudgmentLine_W1"] );
		elseif param.FullComboW2 then
			cf.Number:diffuse( GameColor.Judgment["JudgmentLine_W2"] );
		elseif param.FullComboW3 then
			cf.Number:diffuse( GameColor.Judgment["JudgmentLine_W3"] );
		elseif param.Combo then
			-- Player 1's color is Red, which conflicts with the miss combo.
			-- instead, just diffuse to white for now. -aj
			--c.Number:diffuse(PlayerColor(player));
			cf.Number:diffuse(Color("White"));
			cf.Number:strokecolor(Color("Stealth"));
			cf.Number:stopeffect();
		else
			cf.Number:diffuse(color("#ff0000"));
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
