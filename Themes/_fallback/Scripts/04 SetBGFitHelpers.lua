function BGFitChoiceExample(params)
	-- verify all the parameters, emitting an error for omitted ones.
	local default_params= {
		exname= "16_12_example.png", x= 0, y= 0,
		mini_screen_w= 0, mini_screen_h= 0,
		example_function= bg_fit_functions.BackgroundFitMode_CoverDistort,
			sbg_color= Color.Black, soutline_color= Color.Green
	}
	for name, value in pairs(default_params) do
		if not params[name] then
			lua.ReportScriptError("BGFitChoiceExample not passed a '" .. name ..
															"' param.")
			params[name]= value
		end
	end
	return Def.ActorFrame{
		Name= params.exname, InitCommand= function(self)
			self:xy(params.x, params.y)
		end,
		-- Quad representing the screen, to show behind the example quad.
		Def.Quad{
			Name="mini_scr", InitCommand= function(self)
				self:setsize(params.mini_screen_w, params.mini_screen_h)
				self:diffuse(params.sbg_color)
			end
		},
		-- Sprite representing the example bg.
		Def.Sprite{
			Name="mini_ex", InitCommand= function(self)
				self:LoadBackground(THEME:GetPathG("ScreenSetBGFit", params.exname))
				self:SetTextureFiltering(false)
				params.example_function(self, params.mini_screen_w, params.mini_screen_h)
			end
		},
		-- AMV for the outline of the screen, to show where the screen is so
		-- cropping is visible.
		Def.ActorMultiVertex{
			Name="mini_outline", InitCommand= function(self)
				local hw= params.mini_screen_w / 2
				local hh= params.mini_screen_h / 2
				local verts= {
					{{-hw, -hh, 0}, params.soutline_color},
					{{hw, -hh, 0}, params.soutline_color},
					{{hw, hh, 0}, params.soutline_color},
					{{-hw, hh, 0}, params.soutline_color},
					{{-hw, -hh, 0}, params.soutline_color},
				}
				self:SetVertices(verts)
				self:SetLineWidth(2)
				self:SetDrawState{Mode= "DrawMode_LineStrip"}
			end
		},
		params.example_label,
	}
end

function BGFitInputActor(choices, lose_focus, gain_focus)
	local curr_choice= BackgroundFitMode:Reverse()[PREFSMAN:GetPreference("BackgroundFitMode")] + 1 -- reverse is 0-indexed
	local left_buttons= {MenuLeft= true, MenuUp= true}
	local right_buttons= {MenuRight= true, MenuDown= true}
	local saw_press= false
	if not lose_focus then
		lua.ReportScriptError("BGFitInputActor not given a lose_focus function.")
		lose_focus= function() end
	end
	if not gain_focus then
		lua.ReportScriptError("BGFitInputActor not given a gain_focus function.")
		gain_focus= function() end
	end
	local function check_choice_and_change_focus(focus_func)
		if choices[curr_choice] then
			focus_func(choices[curr_choice])
		else
			lua.ReportScriptError("BGFitInputActor not given a choice actor for the " .. BackgroundFitMode[curr_choice] .. " mode.")
		end
	end
	local function input(event)
		if event.type == "InputEventType_Release" then return false end
		if event.type == "InputEventType_Repeat" and not saw_press then
			return false
		end
		if not event.GameButton then return false end
		saw_press= true
		if left_buttons[event.GameButton] or right_buttons[event.GameButton] then
			check_choice_and_change_focus(lose_focus)
			if left_buttons[event.GameButton] then
				curr_choice= math.max(curr_choice - 1, 1)
			else
				curr_choice= math.min(curr_choice + 1, #choices)
			end
			check_choice_and_change_focus(gain_focus)
			PREFSMAN:SetPreference("BackgroundFitMode", BackgroundFitMode[curr_choice])
			SOUND:PlayOnce(THEME:GetPathS("ScreenSelectMaster", "change"))
		elseif event.GameButton == "Start" or event.GameButton == "Back" then
			SOUND:PlayOnce(THEME:GetPathS("ScreenSelectMaster", "start"))
			SCREENMAN:GetTopScreen():StartTransitioningScreen("SM_GoToNextScreen")
		end
		return false
	end
	return Def.ActorFrame{
		Name= "input_init",
		OnCommand= function(self)
			local screen= SCREENMAN:GetTopScreen()
			screen:AddInputCallback(input)
			choices[curr_choice]:stoptweening()
			choices[curr_choice]:linear(.25)
			choices[curr_choice]:zoom(1.5)
		end
	}
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
