course_stopped_by_pause_menu= false

local prompt_text= {
	Start= THEME:GetString("ScreenGameplay", "GiveUpStartText"),
	Select= THEME:GetString("ScreenGameplay", "GiveUpSelectText"),
	Back= THEME:GetString("ScreenGameplay", "GiveUpBackText"),
}
local prompt_actor= false

local function show_prompt(button)
	prompt_actor:playcommand("Show", {text= prompt_text[button]})
end

local function hide_prompt()
	prompt_actor:playcommand("Hide")
end

local enabled_players= {}
local screen_gameplay= false
local notefields= {}

local menu_height= 410
local menu_width= 346
local menu_x= {[PLAYER_1]= THEME:GetMetric(Var "LoadingScreen","PlayerP1MiscX"), [PLAYER_2]= THEME:GetMetric(Var "LoadingScreen","PlayerP2MiscX")}

local pause_menus= {}
local menu_sounds= {}
local menu_frames= {}
local ignore_next_open= {}

local function close_menu(pn)
	pause_menus[pn]:close_menu()
	pause_menus[pn]:hide()
	menu_frames[pn]:visible(false)
	local stay_paused= false
	for pn, menu in pairs(pause_menus) do
		if not menu.hidden then
			stay_paused= true
		end
	end
	if not stay_paused then
		local fg= screen_gameplay:GetChild("SongForeground")
		if fg then fg:visible(old_fg_visible) end
		local overlay= screen_gameplay:GetChild("Overlay")
		overlay:visible(old_overlay_visible)
		screen_gameplay:PauseGame(false)
	end
end

local function backout(screen)
	screen_gameplay:SetPrevScreenName(screen):SetNextScreenName(screen)
		:begin_backing_out()
end

local function forfeit()
	backout(SelectMusicOrCourse())
end

local function restart_song()
	local tokens_to_add= GAMESTATE:GetNumStagesForCurrentSongAndStepsOrCourse()
	for i, pn in ipairs(GAMESTATE:GetEnabledPlayers()) do
		for t= 1, tokens_to_add do
			GAMESTATE:AddStageToPlayer(pn)
		end
	end
	backout(Branch.GameplayScreen())
end

local function skip_song()
	screen_gameplay:PostScreenMessage("SM_NotesEnded", 0)
end

local function end_course()
	course_stopped_by_pause_menu= true
	screen_gameplay:PostScreenMessage("SM_NotesEnded", 0)
end

local menu_data= get_player_options_menu()

table.insert(menu_data, 1, {"action", "play", function(b, a, pn)
	close_menu(pn)
end})

if GAMESTATE:IsCourseMode() then
	menu_data[#menu_data+1]= {"action", "skip_song", skip_song}
	menu_data[#menu_data+1]= {"action", "forfeit", forfeit}
	menu_data[#menu_data+1]= {"action", "end_course", end_course}
else
	menu_data[#menu_data+1]= {"action", "forfeit", forfeit}
	menu_data[#menu_data+1]= {"action", "restart_song", restart_song}
end

local function show_menu(pn)
	menu_frames[pn]:visible(true)
	pause_menus[pn]:show()
	pause_menus[pn]:open_menu()
end

local function mouse_pn()
	local mx= INPUTFILTER:GetMouseX()
	if mx < _screen.cx then return PLAYER_1 end
	return PLAYER_2
end

local prev_mx= INPUTFILTER:GetMouseX()
local prev_my= INPUTFILTER:GetMouseY()
local function update()
	if not screen_gameplay:IsPaused() then return end
	local mx= INPUTFILTER:GetMouseX()
	local my= INPUTFILTER:GetMouseY()
	if mx == prev_mx and my == prev_my then return end
	local pn= mouse_pn()
	if not pause_menus[pn] then return end
	local sound_name= pause_menus[pn]:update_focus(mx, my)
	nesty_menus.play_menu_sound(menu_sounds, sound_name)
	prev_mx= mx
	prev_my= my
end

local function handle_menu(pn, event)
	local levels_left, sound= pause_menus[pn]:input(event)
	nesty_menus.play_menu_sound(menus_sounds, sound)
	if levels_left and levels_left < 1 then
		close_menu(pn)
		ignore_next_open[pn]= true
	end
end


local function input(event)
	if not screen_gameplay:IsPaused() then return end
	local pn= event.PlayerNumber
	if event.DeviceInput.is_mouse then pn= mouse_pn() end
	local was_ignored= false
	if event.GameButton == "Start" and event.type == "InputEventType_Release" then
		was_ignored= ignore_next_open[pn]
		ignore_next_open[pn]= false
	end
	if not enabled_players[pn] then return end
	if pause_menus[pn].hidden then
		if event.GameButton == "Start" and event.type == "InputEventType_Release" and not was_ignored then
			show_menu(pn)
		end
		return
	else
		handle_menu(pn, event)
	end
end

local main_frame= Def.ActorFrame{
	pause_controller_actor(),
	OnCommand= function(self)
		screen_gameplay= SCREENMAN:GetTopScreen()
		if screen_gameplay:GetName() == "ScreenGameplaySyncMachine" then return end
		menu_sounds= nesty_menus.make_menu_sound_lookup(self)
		for i, pn in ipairs(GAMESTATE:GetEnabledPlayers()) do
			local side= screen_gameplay:GetChild("Player" .. ToEnumShortString(pn))
			if side then
				notefields[pn]= side:GetChild("NoteField")
			end
		end
		for pn, on in pairs(enabled_players) do
			local player_frame= self:GetChild("pause_stuff"..pn)
			pause_menus[pn]:init{
				actor= player_frame:GetChild("menu"), pn= pn,
				data= menu_data, translation_section= "notefield_options"}
			menu_frames[pn]:visible(false)
			pause_menus[pn]:hide()
		end
		screen_gameplay= SCREENMAN:GetTopScreen()
		screen_gameplay:AddInputCallback(input)
		self:SetUpdateFunction(update)
	end,
	MenuValueChangedMessageCommand= function(self, params)
		nesty_menus.handle_menu_refresh_message(params, pause_menus)
	end,
	PlayerHitPauseMessageCommand= function(self, params)
		local overlay= screen_gameplay:GetChild("Overlay")
		old_overlay_visible= overlay:GetVisible()
		overlay:visible(true):hibernate(0)
		local fg= screen_gameplay:GetChild("SongForeground")
		if fg then
			old_fg_visible= fg:GetVisible()
			fg:visible(false)
		end
		screen_gameplay:PauseGame(true)
		ignore_next_open= {}
		show_menu(params.pn)
		hide_prompt()
	end,
	ShowPausePromptMessageCommand= function(self, params)
		show_prompt(params.button)
	end,
	HidePausePromptMessageCommand= function(self)
		hide_prompt()
	end,
	Def.BitmapText{
		Font= "Common Normal", InitCommand= function(self)
			prompt_actor= self
			self:xy(_screen.cx, _screen.h*.75):zoom(.75):diffusealpha(0)
		end,
		ShowCommand= function(self, param)
			self:stoptweening():settext(param.text):accelerate(.25):diffusealpha(1)
				:sleep(1):queuecommand("Hide")
		end,
		HideCommand= function(self)
			self:stoptweening():decelerate(.25):diffusealpha(0)
		end,
	},
}
local sand= nesty_menus.load_typical_menu_sounds()
if sand then
	main_frame[#main_frame+1]= sand
end

for i, pn in ipairs(GAMESTATE:GetEnabledPlayers()) do
	enabled_players[pn]= true
	pause_menus[pn]= setmetatable({}, menu_controller_mt)
	local player_frame= Def.ActorFrame{
		Name= "pause_stuff"..pn, InitCommand= function(self)
			menu_frames[pn]= self
			self:visible(false)
			self:x(menu_x[pn])
		end,
		LoadActor(
			THEME:GetPathG("ScreenOptions", "halfpage")) .. {
			InitCommand= function(self)
				self:xy(0, 360)
			end,
			OnCommand=function(self)
				self:diffusealpha(0):zoomx(0.8):decelerate(0.3):diffusealpha(1):zoomx(1)
			end,
			OffCommand=function(self)
				self:decelerate(0.3):diffusealpha(0)
			end,
		},
		LoadActor(THEME:GetPathG("", "generic_menu.lua"), 1,
							menu_width, menu_height, 1,
							-(menu_width/2), 138, 36)
	}
	main_frame[#main_frame+1]= player_frame
end

for i, pn in ipairs(GAMESTATE:GetEnabledPlayers()) do
	enabled_players[pn]= true
end

return main_frame
