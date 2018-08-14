local PlayerNumber = ...
assert( PlayerNumber )

local bpm_text_zoom = 0.6

local song_bpms= {}
local bpm_text= "??? - ???"
local function format_bpm(bpm)
	return ("%.0f"):format(bpm)
end

-- Courses don't have GetDisplayBpms.
if GAMESTATE:GetCurrentSong() then
	song_bpms= GAMESTATE:GetCurrentSong():GetDisplayBpms()
	song_bpms[1]= math.round(song_bpms[1])
	song_bpms[2]= math.round(song_bpms[2])
	if song_bpms[1] == song_bpms[2] then
		bpm_text= format_bpm(song_bpms[1])
	else
		bpm_text= format_bpm(song_bpms[1]) .. " - " .. format_bpm(song_bpms[2])
	end
end

local t = Def.ActorFrame {
	LoadActor(THEME:GetPathB("_frame","3x1"),"rounded light", 250-16) .. {
		OnCommand=cmd(diffuse,ColorLightTone(PlayerColor(PlayerNumber));diffusealpha,0.8);
	};
	LoadFont("Common Condensed") .. {
		Text=ToEnumShortString(PlayerNumber);
		Name="PlayerShortName",
		InitCommand=cmd(x,-127;maxwidth,32;zoom,0.75),
		OnCommand=cmd(diffuse,ColorDarkTone(PlayerColor(PlayerNumber)))
	},
	LoadFont("_overpass 36px") .. {
		Text=bpm_text;
		Name="BPMRangeOld",
		InitCommand=cmd(x,-60;maxwidth,88/bpm_text_zoom),
		OnCommand=cmd(zoom,bpm_text_zoom;diffuse,ColorDarkTone(PlayerColor(PlayerNumber)))
	},
	LoadActor(THEME:GetPathG("_StepsDisplayListRow","arrow")) .. {
		Name="Seperator",
		InitCommand=cmd(x,4);
		OnCommand=cmd(diffuse,ColorDarkTone(PlayerColor(PlayerNumber)))
	},
	LoadFont("_overpass 36px") .. {
		Text="100 - 200000";
		Text="100 - 200000";
		Name="BPMRangeNew",
		InitCommand= function(self)
			self:x(68):maxwidth(88/bpm_text_zoom):zoom(bpm_text_zoom)
			local speed, mode= GetSpeedModeAndValueFromPoptions(PlayerNumber)
			self:playcommand("SpeedChoiceChanged", {pn= PlayerNumber, mode= mode, speed= speed})
		end,
		BPMWillNotChangeCommand=cmd(stopeffect;diffuse,ColorDarkTone(PlayerColor(PlayerNumber))),
		BPMWillChangeCommand=cmd(diffuseshift;effectcolor1,Color.Black;effectcolor2,ColorMidTone(PlayerColor(PlayerNumber))),
		SpeedChoiceChangedMessageCommand= function(self, param)
			if param.pn ~= PlayerNumber then return end
			local text= ""
			local no_change= true
			if param.mode == "x" then
				if not song_bpms[1] then
					text= "??? - ???"
				elseif song_bpms[1] == song_bpms[2] then
					text= format_bpm(song_bpms[1] * param.speed*.01)
				else
					text= format_bpm(song_bpms[1] * param.speed*.01) .. " - " ..
						format_bpm(song_bpms[2] * param.speed*.01)
				end
				no_change= param.speed == 100
			elseif param.mode == "C" then
				text= param.mode .. param.speed
				no_change= param.speed == song_bpms[2] and song_bpms[1] == song_bpms[2]
			else
				no_change= param.speed == song_bpms[2]
				if song_bpms[1] == song_bpms[2] then
					text= param.mode .. param.speed
				else
					local factor= song_bpms[1] / song_bpms[2]
					text= param.mode .. format_bpm(param.speed * factor) .. " - "
						.. param.mode .. param.speed
				end
			end
			self:settext(text)
			if no_change then
				self:queuecommand("BPMWillNotChange")
			else
				self:queuecommand("BPMWillChange")
			end
		end
	}
}

return t
