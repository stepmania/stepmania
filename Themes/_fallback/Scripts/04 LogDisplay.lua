-- This is a little fake actor class meant for displaying lines of a log.
-- It's placed inside Def, but it's actually just an ActorFrame with some
--   children and special commands.

-- LogDisplay listens for several messages that control its behavior.
-- The name given to the LogDisplay is added to the name of the message to
--   make the messages unique to this LogDisplay.  So to issue a command to
--   the LogDisplay, broadcast the message "<command><name>".
--   For a LogDisplay named "Foo", you would broadcase "Foo" to add to the
--   log, "ToggleFoo" to toggle whether it's shown or hidden, and so on.

-- Some of the control messages take tables of args.
-- Arg elements of the form "name= type" are named, and must be a value of
--   the given type.
-- Arg elements of the form "name" are unnamed.

-- The messages are:
--   <name>:  params= {message= string, dont_show= bool}
--     Adds message to the log to be shown when revealing.
--     If dont_show is false, then Show will be automatically executed with
--     auto_hide= true.
--     Messages with line breaks will take up multiple lines.
--     The newest message in the log has index 1.
--   Toggle:  No params
--     Executes either Show or Hide, depending on the current state.
--   Show:  params= {range= {last_message, lines}, auto_hide= bool}
--     Shows the messages currently in the log.
--     range[1] is the index of the last message to show.
--     range[2] is the max number of lines to show.
--     range[1] defaults to a cleavage shot.
--     range[2] defaults to MaxDisplayLines.
--     If auto_hide is non-nil, Hide will be execuated after a short time.
--     If auto_hide is a number, it will be used as the time to wait before
--       hiding.  Otherwise, the Times.show will be used.
--   Query:  No params
--     Broadcasts "<name>Response" message with a table containing:
--       {log_len= number, hidden= bool}
--       log_len is the number of messages currently in the log.
--       hidden is whether the LogDisplay is currently hidden.
--   Hide:  No params
--     Hides the LogDisplay.
--   Clear:  params= {messages}
--     Clears the messages in the log.  If messages is a number, only clears
--     that many messages, oldest first.

-- params to Def.LogDisplay():
-- {
--   Name= string
--     The name will be used as the name of the main ActorFrame and to
--     control what messages the LogDisplay listens for.
--   MaxLogLen= number
--     The maximum number of messages to store in the log.
--   MaxDisplayLines= number
--     The maximum number of lines that can be displayed at a time.
--   ReplaceLinesWhenHidden= bool
--     If this is true, any messages recieved while the LogDisplay is hidden
--     will replace the message currently at the front of the log.
--     If this is false, messages recieved while the LogDisplay is hidden
--     will push other messages back.  Messages pushed past MaxLines will be
--     removed.
--   IgnoreIdentical= bool
--     If true, then a message that is identical to one already in the log
--     will be ignored.
--   Font= font name
--     The name of the font to use.  This will be passed to THEME:GetPathF,
--     so it should not be a path.
--   LineHeight= number
--     The height in pixels to use between lines.
--   LineWidth= number
--     The width that lines should be limited to.
--   TextZoom= number
--     The zoom factor to apply to the text.
--   TextColor= color
--     The color to use for the text.
--   Times= {show= number, hide= number}
--     show is the amount of time to wait before automatically hiding when
--     Show is executed with AutoHide == true.  It's passed as a table, so
--     if you keep a copy of the table, you should be able to modify it to
--     change the time.
--     hide is used as the time for the default hide command to hide.
--   Indent= number
--     The amount in pixels to indent lines that were part of a multi-line
--     message.
--   Hide= function(self)
--     This command will be executed when the LogDisplay needs to be hidden.
--     Hide will also be executed during the InitCommand, so the
--     LogDisplay will start in the hidden state.
--   Show= function(self, Lines)
--     This command will be executed when the LogDisplay needs to be shown.
--     Lines is the number of lines to show.
-- }
-- Any actors placed in params will be used and drawn behind the text.
-- If there are no actors in params, a quad filling the area will be used.
-- Reasonable defaults are provided for everything except Name.  If Name is
--   blank, you get nothing.  Defaults assume the LogDisplay should fill the
--   screen when in use.

-- Below is the implementation of the above features.

local log_display_mt= {
	__index= {
		create_actors= function(self, params)
			self.name= params.Name
			self.font= params.Font or "Common Normal"
			self.line_height= params.LineHeight or 12
			self.line_width= params.LineWidth or SCREEN_WIDTH
			self.text_zoom= params.TextZoom or .5
			self.text_color= params.TextColor or color("#93a1a1")
			self.max_lines= params.MaxLines or SCREEN_HEIGHT / self.line_height
			self.max_log= params.MaxLogLen or self.max_lines
			self.indent= params.Indent or 8
			self.times= params.Times
			self.param_hide= params.Hide or
				function(subself)
					subself:linear(params.Times.hide)
					subself:y(-SCREEN_HEIGHT)
				end
			self.param_show= params.Show or
				function(subself, lines)
					subself:visible(true)
					subself:linear(params.Times.hide)
					local cover= math.min(self.line_height * (lines+.5), SCREEN_HEIGHT)
					subself:y(-SCREEN_HEIGHT + cover)
				end

			self.text_actors= {}
			self.message_log= {}
			self.messes_since_update= 0

			local name_mess= self.name .. "MessageCommand"
			local args= {
				Name= self.name,
				InitCommand= function(subself)
					self.container= subself; -- This semicolon ends this statement.
					-- Without it, the next would be ambiguous syntax.
					-- Let the InitCommand passed in params do something.
					(params.InitCommand or function() end)(subself)
					self:hide()
				end,
				OffCommand= function(subself)
					subself:visible(false)
					for i, actor in ipairs(self.text_actors) do
						actor:visible(false)
					end
				end,
				[name_mess]= function(subself, mess)
					if not PREFSMAN:GetPreference("ShowThemeErrors")
					and self.name == "ScriptError" then
						subself:visible(false)
						return
					end
					if not mess.message then return end
					if self.messes_since_update > self.max_log then return end
					-- Long ago, someone decided that "::" should be an alias for "\n"
					-- and hardcoded it into BitmapText.
					local message= tostring(mess.message):gsub("::", ":")
					if params.IgnoreIdentical and not self.hidden then
						for i, prevmess in ipairs(self.message_log) do
							if message == prevmess then return end
						end
					end
					if params.ReplaceLinesWhenHidden and self.hidden then
						self:clear()
						self.message_log[1]= message
					else
						table.insert(self.message_log, 1, message)
						if #self.message_log > self.max_log then
							table.remove(self.message_log)
						end
					end
					if not mess.dont_show or not self.hidden then
						self.messes_since_update= self.messes_since_update + 1
						self.hidden= false
						subself:stoptweening()
						subself:queuecommand("Update")
					end
				end,
				["Toggle" .. name_mess]= function(subself)
					if self.hidden then
						self:show()
					else
						self:hide()
					end
				end,
				["Hide" .. name_mess]= function(subself)
					self:hide()
				end,
				["Show" .. name_mess]= function(subself, mess)
					self:show(mess.range, mess.auto_hide)
				end,
				["Clear" .. name_mess]= function(subself, mess)
					self:clear(mess[1])
				end,
				UpdateCommand= function(subself)
					self:show(nil, true)
					self.messes_since_update= 0
				end,
				LoadFont(self.font) ..
					{
						Name="WidthTester",
						InitCommand= function(subself)
							self.width_tester= subself
							subself:zoom(self.text_zoom)
							subself:visible(false)
						end
					}
			}
			if #params == 0 then
				args[#args+1]= Def.Quad {
					Name= "Logbg",
					InitCommand= function(subself)
						subself:setsize(self.line_width, self.line_height*self.max_lines)
						subself:horizalign(left)
						subself:vertalign(top)
						subself:diffuse(color("#002b36"))
						subself:diffusealpha(.85)
					end,
				}
			else
				-- Add bg actors passed through params.
				for i, actor in ipairs(params) do
					args[#args+1]= actor
				end
			end
			-- Add commands passed through params.
			for name, command in pairs(params) do
				if type(name) == "string" and name:find("Command") and not args[name] then
					args[name]= command
				end
			end
			for i= 1, self.max_lines do
				args[#args+1]= LoadFont(self.font) ..
					{
						Name="Text" .. i,
						InitCommand= function(subself)
							-- Put them in the list in reverse order so the ones at the
							-- bottom of the screen are used first.
							self.text_actors[self.max_lines-i+1]= subself
							subself:horizalign(left)
							subself:vertalign(top)
							subself:y(self.line_height * (i-1))
							subself:zoom(self.text_zoom)
							subself:diffuse(self.text_color)
							subself:visible(false)
						end,
						OffCommand= cmd(visible,false),
					}
			end
			return Def.ActorFrame(args)
		end,
		hide= function(self)
			self.container:stoptweening()
			self.param_hide(self.container)
			self.container:queuecommand("Off")
			self.hidden= true
		end,
		show= function(self, range, auto_hide)
			if not range then range= {} end
			local start= range[1] or 1
			local lmax= range[2] or self.max_lines
			local indented_lines= {}
			local next_message= start
			local num_lines= 0
			while num_lines < lmax and self.message_log[next_message] do
				self.width_tester:settext(self.message_log[next_message])
				local lines_to_add= convert_text_to_indented_lines(
					self.width_tester, self.indent, self.line_width, self.text_zoom)
				indented_lines[#indented_lines+1]= lines_to_add
				num_lines= num_lines + #lines_to_add
				next_message= next_message + 1
			end
			local use_next= 1
			local used= 0
			for i, mess in ipairs(indented_lines) do
				for l, line in ipairs(mess) do
					-- Start at the end of the mess because the text actors are in
					-- reverse order.
					local ind= use_next + (#mess - l)
					local actor= self.text_actors[ind]
					if actor and used <= lmax then
						actor:settext(line[2])
						local indent= 8 + self.indent * line[1]
						local lw= self.line_width - (indent + 8)
						width_limit_text(actor, lw, self.text_zoom)
						actor:x(indent)
						actor:visible(true)
						used= used + 1
					end
				end
				use_next= use_next + #mess
			end
			self.container:stoptweening()
			self.param_show(self.container, used)
			self.hidden= false
			if auto_hide then
				self.container:sleep(self.times.show)
				self.container:queuecommand("Hide" .. self.name)
			end
		end,
		clear= function(self, messes)
			if #self.message_log < 1 then return end
			if not messes then
				self.message_log= {}
			else
				for i= 1, messes do
					table.remove(self.message_log)
					if #self.message_log < 1 then break end
				end
			end
		end
}}

function Def.LogDisplay(params)
	if type(params.Name) ~= "string" or params.Name == "" then
		ReportScriptError("Cannot create a LogDisplay without a name.")
		return nil
	end

	if not params.Times then params.Times= {show= 4, hide= .125} end
	if not params.Times.show then params.Times.show= 4 end
	if not params.Times.hide then params.Times.hide= .125 end

	local new_log_display= setmetatable({}, log_display_mt)
	_G[params.Name .. "LogDisplay"]= new_log_display
	return new_log_display:create_actors(params)
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
