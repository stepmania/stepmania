-- This is an actor class that implements functionality for having a numpad
-- that is used to enter a number.
-- The parameters for customizing its appearance will be discussed here.  For
-- an example, see Docs/ThemerDocs/Examples/Example_Actors/NumPadEntry.lua

-- This actor handles input, so it has to be used a bit differently from
-- normal actors.  Instead of just putting it inside an ActorFrame like
-- normal, you must follow these steps:
-- 1.  Call the function "new_numpad_entry" and store the result in a local
--   variable.  This creates the NumPadEntry that will handle the logic and
--   input of entering a number.  Ex:
--     local entry_pad= new_numpad_entry(params)
-- 2.  Call the "create_actors" function of the NumPadEntry you created and
--   put the actors it returns inside your ActorFrame.  Ex:
--     Def.ActorFrame{ entry_pad:create_actors(params) }
-- 3.  Set up an input callback for recieving input and pass it to the
--   NumPadEntry you created.  Check the return value of the handle_input
--   function to see when the player has finished entering the number.  Ex:
--     local function input(event)
--       if event.type ~= "InputEventType_Release" then
--         local entry_done= entry_pad:handle_input(event.button)
--       end
--     end
-- 4.  If handle_input returns true, the player has finished entering the
--   number and you can do something with the value.  Each NumPadEntry sets
--   its "done" field to true when the player hits the done button.  Ex:
--   -- (continued from above)
--     if entry_done then
--       Trace("Player entered value: " .. entry_pad.value)
--     end

-- Params explanation:
-- 
-- "params" is a table containing the parameters used to customize
-- NumPadEntry.
-- 
-- Almost all parameters are optional except for the Name.
-- 
-- Some parameters are interdependent, so if you include one, you should
-- include the ones it is interdependent with to make sure they make sense
-- together.
-- 
-- The parameters can be passed to either new_numpad_entry() or to
-- create_actors().
-- 
-- Parameters passed to create_actors() are combined with the parameters
-- passed to new_numpad_entry().
-- 
-- If the same parameter is passed to both, the one passed to create_actors()
-- overrides the one passed to new_numpad_entry().
-- 
-- NumPadEntry is like an ActorFrame, you can put actors in params and they
-- will be inside it, drawn before (under) everything else.
-- 
-- You can also include custom commands for the NumPadEntry actor and they
-- will be part of the actor returned by create_actors().
-- 
-- Some parameters are custom actors that fill a role.  They must support
-- certain commands that will be used to carry out their role.
-- Parameter listing:
-- {
--   Name= "foo", -- A unique string you will recognize later.
--
--   -- button_positions, rows, and columns are used to positioning the
--   -- buttons and moving the cursor.  Each set of "rows" number of entries
--   -- in button_positions will be considered one row, and the cursor will
--   -- be wrapped to the beginning/end of the row or moved to a new row or
--   -- column as appropriate when input is handled.
--
--   -- Optional.  A table of positions for the buttons.  Each position is a
--   -- table containing the x, y, and optional z of that button.
--   button_positions= {{x, y, z}, {x, y, z}, ...},
--   rows= 4, -- Optional.  The number of rows on the pad.
--   columns= 4, -- Optional.  The number of columns on the pad.
--
--   -- Optional.  The button that the cursor starts on.  The default is the
--   -- button in the middle of the pad.
--   cursor_pos= 5,
--
--   done_text= "&start;", -- Optional.  The text used for the "Done" button.
--   back_text= "&leftarrow;", -- Optional.  The text used for the
--   -- "Backspace" button.
--
--   -- Optional.  The values of the buttons.  One of them should have
--   -- done_text as a value and one should have back_text as a value.  The
--   -- default is for a standard 12 button numpad, numbers 0-9, done, and
--   -- backspace.
--   button_values= {7, 8, 9, 4, 5, 6, 1, 2, 3, 0, done_text, back_text},
--
--   -- button_positions, button_values, rows, columns, and cursor_pos are
--   -- all interdependent.  If you pass one, you should pass them all.
--
--   -- Optional.  The amount to multiply by when adding a digit.
--   digit_scale= 10,
--   -- Optional.  If the player tries to set the value above this amount,
--   -- InvalidValueCommand will be played on the actor.  Default allows any
--   -- value.
--   max_value= 300,
--   -- Optional.  When the current value is above this, the cursor will be
--   -- moved to the done button.  Default is to never automatically move the
--   -- cursor.
--   auto_done_value= 100,
--
--   -- Optional.  The various default actors will use this font if a
--   -- specific for the actor is not passed in and they are not replaced by
--   -- custom actors.  Default is "Common Normal".
--   Font= "Common Normal",
--
--   -- Commands:  These are some commands that are recommended, but not
--   -- required.
--
--   -- Optional.  You can pass an InitCommand to set the position and stuff.
--   InitCommand= function() end,
--
--   -- Optional.  InvalidValueCommand will be executed when the player tries
--   -- to enter an invalid value.  params is a table containing the value.
--   -- The default plays the common invalid sound.
--   InvalidValueCommand= function(self, params) Trace(params[1]) end,
--
--   -- Optional.  EntryDoneCommand will be executed when the player presses
--   -- the done button.  params is a table containing the value.  The
--   -- default does nothing.
--   EntryDoneCommand= function(self, params) Trace(params[1]) end,
--
--   -- Actors:  These are the optional custom actors you can provide to
--   -- change the appearance of each part of the numpad.
--   -- Each of them has a default that can be customized in minor ways, or
--   -- you can provide a full actor to replace the default.
--
--   -- Optional.  The color the default cursor will use.  Unused if you pass
--   -- in a custom cursor.
--   cursor_color= Color.black,
--   -- Optional.  The actor used for the cursor.  Should have the Move and
--   -- Fit commands.  Default is a simple quad that moves and changes its
--   -- width.
--   -- When a cursor movement occurs, MoveCommand will be executed, followed
--   -- by FitCommand.  Both are executed by playcommand so they can be
--   -- passed parameters.  Default Move and Fit commands do nothing.
--   cursor= Def.Quad{
--     -- param is the position to move to.  Be sure to avoid a tween
--     -- overflow.
--     MoveCommand= function(self, param) end,
--     -- param is the actor for the button the cursor is moving to.
--     FitCommand= function(self, param) end,
--   },
--   -- Required.  Where the cursor should be placed in the ActorFrame of the
--   -- numpad.  "first" means the cursor will be placed first, and thus
--   -- under all the buttons.  "last" means the cursor will be placed last,
--   -- and thus above all the buttons.  nil means no cursor.
--   cursor_draw= "first",
--
--   -- Optional.  The font used by the default button actor.  Unused if you
--   -- pass in a custom button actor.
--   button_font= "Common Normal",
--   -- Optional.  The color used by the default button actor.  Unused if you
--   -- pass in a custom button actor.  Default is White.
--   button_color= Color.White,
--   -- Optional.  A template for the actor that will be used for each button.
--   -- This template will be duplicated for each button.
--   -- Do not provide a name in the button template, each button will be
--   -- named uniquely by the numpad like this:  Name= "num"..index
--   -- Do not attempt to do any positioning in the InitCommand for your
--   -- button actor, positioning will be handlded by the numpad, using the
--   -- positions provided in button_positions.
--   -- GainFocus, LoseFocus, and Press are optional commands for handling
--   -- their respective events.
--   button= Def.BitmapText{
--     InitCommand= cmd(diffuse, Color.White),
--     -- SetCommand is executed after InitCommand to set the value for the
--     -- button.  param is a table containing a value from button_values.
--     SetCommand= function(self, param)
--       self:settext(param[1])
--     end,
--     -- GainFocusCommand is executed when the cursor moves onto the button.
--     GainFocusCommand= cmd(diffuse, Color.Red),
--     -- LoseFocusCommand is executed when the cursor moves off the button.
--     LoseFocusCommand= cmd(diffuse, Color.White),
--     -- PressCommand is executed when the button is pressed.
--     PressCommand= cmd(stoptweening; linear,.1; zoom,2; linear.1; zoom,1)
--   },
--
--   -- Optional.  The position to place the default value text at.  Position
--   -- is a table of x, y, and optional z.  Unused if you pass in a custom
--   -- value actor.
--   value_pos= {0, -48},
--   -- Optional.  The font to use for the default value actor.
--   value_font= "Common Normal",
--   -- Optional.  The color for the default value text.  Unused if you pass
--   -- in a custom value actor.  Default is White.
--   value_color= Color.White,
--   -- Optional.  The actor used for displaying the value the player has
--   -- entered so far.  Default is a simple BitmapText.
--   value= Def.BitmapText{
--     Font= "Common Normal",
--     -- InitCommand should set the position you want if you pass a custom
--     -- actor.
--     InitCommand= cmd(xy, 0, -48),
--     -- SetCommand will be executed when a button is pressed and the value
--     -- is changed.  param is a table containing the new value.
--     SetCommand= function(self, param) self:settext(param[1]) end
--   },
--
--   -- Optional.  The position to place the default prompt actor at.
--   -- Position is a table of x, y, and optional z.
--   prompt_pos= {0, -72},
--   -- Optional.  The font to use for the default prompt actor.
--   prompt_font= "Common Normal",
--   -- Optional.  The color used by the default prompt actor.
--   prompt_color= Color.White,
--   -- Optional.  The text used by the default prompt actor.  Default is "".
--   prompt_text= "",
--   -- Optional.  The actor to use for displaying the prompt.  Any actor you
--   -- want.  It should support a Set command if the NumPadEntry is on a
--   -- screen where it will be reused for different numbers.
--   prompt= Def.BitmapText{
--     Font= "Common Normal",
--     -- InitCommand should set the position you want if you pass a custom
--     -- actor.
--     InitCommand= cmd(xy, 0, -48),
--     -- SetCommand will be executed when a button is pressed and the value
--     -- is changed.  param is a table containing the new value.
--     SetCommand= function(self, param) self:settext(param[1]) end
--   },
-- }

local function noop() end

local function add_default_commands_to_actor(default_set, actor)
	for i, command_name in ipairs(default_set) do
		if not actor[command_name] then actor[command_name]= noop end
	end
end

local function pos_to_cr(pos, columns)
	return {((pos-1) % columns)+1, math.ceil(pos / columns)}
end

local function cr_to_pos(cr, columns)
	return ((cr[2] - 1) * columns) + (cr[1])
end

local numpad_entry_mt= {
	__index= {
		init= function(self, params)
			self.init_params= params
			return self
		end,
		create_actors= function(self, params)
			params= params or {}
			if self.init_params then
				for name, param in pairs(self.init_params) do
					if not params[name] then params[name]= param end
				end
			end
			if not params.Name then
				error("NumPadEntry(" .. self.name .. "):  Every actor should have a Name.")
			end
			self.name= params.Name
			self.button_poses= params.button_positions or
				{{-24, -24}, {0, -24}, {24, -24},
				{-24, 0},   {0, 0},   {24, 0},
				{-24, 24}, {0, 24},   {24, 24},
				{-24, 48}, {0, 48},   {24, 48}}
			self.rows= params.rows or 4
			self.columns= params.columns or 3
			if #self.button_poses ~= self.rows * self.columns then
				error("NumpadEntry(" .. self.name .. "):  Number of buttons does not match rows * columns.")
			end
			local default_start= cr_to_pos(
				{math.ceil(self.columns/2), math.ceil(self.rows/2)}, self.columns)
			self.cursor_start= params.cursor_start or default_start
			self.cursor_pos= self.cursor_start
			self.done_text= params.done_text or "&start;"
			self.back_text= params.back_text or "&leftarrow;"
			self.button_values= params.button_values or
				{7, 8, 9, 4, 5, 6, 1, 2, 3, 0, self.done_text, self.back_text}
			if #self.button_values ~= #self.button_poses then
				error("NumpadEntry(" .. self.name .. "):  Number of button values does not match number of button positions.")
			end
			self.value= 0
			self.digit_scale= params.digit_scale or 10
			self.max_value= params.max_value
			self.auto_done_value= params.auto_done_value
			if MonthOfYear() == 3 and DayOfMonth() == 1 and PREFSMAN:GetPreference("EasterEggs") then
				for i= 1, #self.button_values do
					local a= math.random(1, #self.button_values)
					local b= math.random(1, #self.button_values)
					self.button_values[a], self.button_values[b]= self.button_values[b], self.button_values[a]
				end
			end
			self.done_pos= (self.rows * self.columns) - 1
			for i, val in ipairs(self.button_values) do
				if val == self.done_text then
					self.done_pos= i
					break
				end
			end
			local args= {
				Name= self.name, InitCommand= function(subself)
					(params.InitCommand or noop)(subself)
					self:update_cursor()
					self.container= subself
				end
			}
			for i, actor in ipairs(params) do
				args[#args+1]= actor
			end
			for name, command in pairs(params) do
				if type(name) == "string" and name:find("Command") and not args[name] then
					args[name]= command
				end
			end
			if not args.InvalidValueCommand then
				args.InvalidValueCommand= function(subself)
					SOUND:PlayOnce(THEME:GetPathS("Common", "Invalid"))
				end
			end
			add_default_commands_to_actor({"EntryDoneCommand"}, args)
			local default_cursor= Def.Quad{
				Name= "cursor", InitCommand= cmd(setsize, 16, 24; diffuse, params.cursor_color or Color.Black),
				MoveCommand= function(subself, param)
					subself:stoptweening()
					subself:linear(.1)
					subself:xy(param[1], param[2])
					if param[3] then subself:z(param[3]) end
				end,
				FitCommand= function(subself, param)
					subself:SetWidth(param:GetWidth())
				end
			}
			local cursor_template= params.cursor or default_cursor
			local cursor_init= cursor_template.InitCommand or noop
			cursor_template.InitCommand= function(subself)
				self.cursor= subself
				cursor_init(subself)
			end
			add_default_commands_to_actor({"FitCommand", "MoveCommand"}, cursor_template)
			if params.cursor_draw == "first" then
				args[#args+1]= cursor_template
			end
			self.button_actors= {}
			local default_bat_commands= {
				"GainFocusCommand", "LoseFocusCommand", "PressCommand"}
			local default_bat= Def.BitmapText{
				Font= params.button_font or params.Font or "Common Normal",
				InitCommand= cmd(diffuse, params.button_color or Color.White),
				SetCommand= function(subself, param)
					subself:settext(param[1])
				end
			}
			local ba_template= params.button or default_bat
			add_default_commands_to_actor(default_bat_commands, ba_template)
			local bainit= ba_template.InitCommand or noop
			for i, pos in ipairs(self.button_poses) do
				local actor= DeepCopy(ba_template)
				actor.InitCommand= function(subself)
					self.button_actors[i]= subself
					subself:xy(pos[1], pos[2])
					if pos[3] then subself:z(pos[3]) end
					bainit(subself)
					subself:playcommand("Set", {self.button_values[i]})
				end
				actor.Name= "num"..i
				args[#args+1]= actor
			end
			local vat_pos= params.value_pos or {0, -48}
			local default_vat= Def.BitmapText{
				Name= "value",
				Font= params.value_font or params.Font or "Common Normal",
				InitCommand= function(subself)
					subself:xy(vat_pos[1], vat_pos[2])
					if vat_pos[3] then subself:z(vat_pos[3]) end
					subself:diffuse(params.value_color or Color.White)
				end,
				SetCommand= function(subself, param) subself:settext(param[1]) end}
			local va_template= params.value or default_vat
			add_default_commands_to_actor({"SetCommand"}, va_template)
			local vainit= va_template.InitCommand or noop
			va_template.InitCommand= function(subself)
				self.value_actor= subself
				vainit(subself)
				subself:playcommand("Set", {self.value})
			end
			args[#args+1]= va_template
			local prompt_pos= params.prompt_pos or {0, -72}
			local default_prompt= Def.BitmapText{
				Name= "prompt",
				Font= params.prompt_font or params.Font or "Common Normal",
				InitCommand= cmd(xy, prompt_pos[1], prompt_pos[2];
					diffuse, params.prompt_color or Color.White),
				Text= params.prompt_text or "",
				SetCommand= function(subself, param) subself:settext(param[1]) end}
			local prompt_template= params.prompt or default_prompt
			local prompt_init= prompt_template.InitCommand or noop
			add_default_commands_to_actor({"SetCommand"}, prompt_template)
			prompt_template.InitCommand= function(subself)
				self.prompt_actor= subself
				prompt_init(subself)
			end
			args[#args+1]= prompt_template
			if params.cursor_draw == "last" then
				args[#args+1]= cursor_template
			end
			return Def.ActorFrame(args)
		end,
		update_cursor= function(self, new_pos)
			if new_pos then
				self.button_actors[self.cursor_pos]:playcommand("LoseFocus")
				self.cursor_pos= new_pos
			end
			self.button_actors[self.cursor_pos]:playcommand("GainFocus")
			if not self.cursor then return end
			self.cursor:playcommand("Move", self.button_poses[self.cursor_pos])
			self.cursor:playcommand("Fit", self.button_actors[self.cursor_pos])
		end,
		handle_input= function(self, button)
			if button == "Start" then
				self.button_actors[self.cursor_pos]:playcommand("Press")
				local num= self.button_values[self.cursor_pos]
				local as_num= tonumber(num)
				if as_num then
					local new_value= (self.value * self.digit_scale) + as_num
					if self.max_value and new_value > self.max_value then
						self.container:playcommand("InvalidValue", {new_value})
					else
						self.value= new_value
						if self.auto_done_value and new_value > self.auto_done_value then
							self:update_cursor(self.done_pos)
						end
						self.value_actor:playcommand("Set", {self.value})
					end
				else
					if num == self.done_text then
						self.container:playcommand("EntryDone", {self.value})
						self.done= true
						return true
					elseif num == self.back_text then
						self.value= math.floor(self.value / self.digit_scale)
						self.value_actor:playcommand("Set", {self.value})
					end
				end
			else
				local cr_pos= pos_to_cr(self.cursor_pos, self.columns)
				local button_motions= {
					Left= {-1, 0}, Right= {1, 0}, Up= {0, -1}, Down= {0, 1}}
				button_motions.MenuLeft= button_motions.Left
				button_motions.MenuRight= button_motions.Right
				button_motions.MenuUp= button_motions.Up
				button_motions.MenuDown= button_motions.Down
				local motion= button_motions[button] -- Come on, do the loca-motion!
				if motion then
					cr_pos[1]= cr_pos[1] + motion[1]
					cr_pos[2]= cr_pos[2] + motion[2]
					if cr_pos[1] < 1 then
						cr_pos[1]= self.columns
						if button == "MenuLeft" then
							cr_pos[2]= cr_pos[2] - 1
						end
					end
					if cr_pos[1] > self.columns then
						cr_pos[1]= 1
						if button == "MenuRight" then
							cr_pos[2]= cr_pos[2] + 1
						end
					end
					if cr_pos[2] < 1 then cr_pos[2]= self.rows end
					if cr_pos[2] > self.rows then cr_pos[2]= 1 end
					self:update_cursor(cr_to_pos(cr_pos, self.columns))
				end
			end
		end
}}

function new_numpad_entry(params)
	return setmetatable({}, numpad_entry_mt):init(params)
end

--[[
Copyright © 2014 Eric Reese / Kyzentun
All rights reserved.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
]]
