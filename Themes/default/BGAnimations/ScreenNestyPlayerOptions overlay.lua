local menu_height= 410
local menu_width= 346
local menu_x= {
	[PLAYER_1]= _screen.w * .25,
	[PLAYER_2]= _screen.w * .75,
}

local player_ready= {}
local ready_indicators= {}
local explanations= {}

local prev_explanation= {}
local function update_explanation(cursor_item, pn)
	if cursor_item then
		if not cursor_item.info then return end
		local new_expl= cursor_item.info.name 
		local expl_com= "change_explanation"
		if cursor_item.info.explanation then
			new_expl= cursor_item.info.explanation
			expl_com= "translated_explanation"
		end
		if new_expl ~= prev_explanation[pn] then
			prev_explanation[pn]= new_expl
			explanations[pn]:playcommand(expl_com, {text= new_expl})
		end
		if not cursor_item.info then return end
	end
end

local function exit_if_both_ready()
	for i, pn in ipairs(GAMESTATE:GetHumanPlayers()) do
		if not player_ready[pn] then return end
	end
	SCREENMAN:GetTopScreen():StartTransitioningScreen("SM_GoToNextScreen")
end

-- The menu system passes in big and arg, but this doesn't really need them.
-- big is true when the player has held the button for 10 or more repeats.
-- arg is custom info in the menu data. Since the menu data didn't set it,
-- it's blank.
-- -Kyz
local function change_ready(big, arg, pn)
	player_ready[pn]= true
	ready_indicators[pn]:playcommand("show_ready")
	exit_if_both_ready()
end

local function back_to_ssm(big, arg, pn)
	SCREENMAN:GetTopScreen():StartTransitioningScreen("SM_GoToPrevScreen")
end

local menu_data= get_player_options_menu()
-- Play and back options inserted here because breaking up the menu data
-- section with the ready/back functiong bothered me. -Kyz
table.insert(menu_data, 1, {"action", "play_song", change_ready})
menu_data[#menu_data+1]= {"action", "back_to_ssm", back_to_ssm}

local menu_actors= {}
local frame= Def.ActorFrame{}

for i, pn in ipairs(GAMESTATE:GetHumanPlayers()) do
	menu_actors[pn]= LoadActor(THEME:GetPathG("", "generic_menu.lua"), 1, 352, menu_height, 1, menu_x[pn]-(menu_width/2), 138, 36)
	frame[#frame+1]= LoadActor(
		THEME:GetPathG("ScreenOptions", "halfpage")) .. {
		InitCommand= function(self)
			self:xy(menu_x[pn], 360)
		end;
		OnCommand=function(self)
			self:diffusealpha(0):zoomx(0.8):decelerate(0.3):diffusealpha(1):zoomx(1)
		end;
		OffCommand=function(self)
			self:decelerate(0.3):diffusealpha(0)
		end;
	}
end

local menu_layer, menu_controllers= nesty_menus.make_menu_actors{
	actors= menu_actors,
	data= menu_data,
	input_mode= theme_config:get_data().menu_mode,
	item_change_callback= update_explanation,
	exit_callback= back_to_ssm,
	translation_section= "notefield_options",
	-- Outlines clickable and focusable areas so you can tune them.
	--with_click_debug= true,
}

frame[#frame+1]= menu_layer

for i, pn in ipairs(GAMESTATE:GetHumanPlayers()) do
	frame[#frame+1]= Def.BitmapText{
		Font= "Common Normal", InitCommand= function(self)
			explanations[pn]= self
			self:xy(menu_x[pn] - (menu_width / 2), _screen.cy+178)
				:diffuse(ColorDarkTone((PlayerColor(pn)))):horizalign(left):vertalign(top)		
				:wrapwidthpixels(menu_width / .8):zoom(.75)
				:horizalign(left)
		end,
		change_explanationCommand= function(self, param)
			local text= ""
			if THEME:HasString("notefield_explanations", param.text) then
				text= THEME:GetString("notefield_explanations", param.text)
			end
			self:playcommand("translated_explanation", {text= text})
		end,
		translated_explanationCommand= function(self, param)
			self:stoptweening():settext(param.text):cropright(1):linear(.5):cropright(0)
		end,
	}
	frame[#frame+1]= LoadActor(THEME:GetPathG("NestyOptions", "ready")) .. {
		InitCommand= function(self)
			ready_indicators[pn]= self
			self:xy(menu_x[pn], 581):zoom(0.4):diffusealpha(0)
		end,
		show_readyCommand= function(self)
				self:stoptweening():bouncebegin(.2):diffusealpha(1):zoom(0.6):bounceend(0.2):zoom(0.5)
		end,
		hide_readyCommand= function(self)
				self:stoptweening():decelerate(.3):diffusealpha(0):zoom(0.4)
		end,
		OffCommand=function(self)
				self:finishtweening():bouncebegin(.2):zoom(0.6):bounceend(0.2):zoom(0):diffusealpha(0)
		end,
	}
	local metrics_name = "PlayerNameplate" .. ToEnumShortString(pn)
	frame[#frame+1] = LoadActor(
		THEME:GetPathG("ScreenPlayerOptions", "PlayerNameplate"), pn) .. {
		InitCommand=function(self)
			self:name(metrics_name)
			ActorUtil.LoadAllCommandsAndSetXY(self,"ScreenPlayerOptions")
			self:x(menu_x[pn])
		end
	}
end

return frame
