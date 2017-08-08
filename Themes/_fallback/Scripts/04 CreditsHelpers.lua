local function Fooled()
	local phrases = {
		"hornswaggled",
		"bamboozled",
		"hoodwinked",
		"swindled",
		"duped",
		"hoaxed",
		"fleeced",
		"shafted",
		"caboodled",
		"beguiled",
		"finagled",
		"two-timed",
		"suckered",
		"flimflammed",
		"generous"
	}
	return phrases[math.random(#phrases)]
end

local line_height= 30 -- so that actor logos can use it.

local stepmania_credits= {
	{
		name= "the spinal shark collective (project lead)",
		"AJ Kelly as freem",
		"Jonathan Payne (Midiman)",
		"Colby Klein (shakesoda)",
	},
	{
		name= "sm-ssc Team",
		"Jason Felds (wolfman2000)", -- Timing Segments, Split Timing, optimization
		"Thai Pangsakulyanont (theDtTvB)", -- BMS, Split Timing, optimization
		"Alberto Ramos (Daisuke Master)",
		"Jack Walstrom (FSX)",
	},
	{
		name= "StepMania Team",
		"Chris Danford",
		"Glenn Maynard",
		"Steve Checkoway",
		-- and various other contributors
	},
	{
		name= "OpenITG Team",
		"infamouspat",
		"Mark Cannon (vyhd)",
	},
	{
		name= "Translators",
		{type= "subsection", name= "Pre-5.0.5"},
		"John Reactor (Polish)",
		"DHalens (Spanish)",
		"@Niler_jp (Japanese)",
		"Deamon007 (Dutch)",
		{type= "subsection", name= "5.0.5 update"},
		"Kevin O. (Thumbsy) (Dutch)",
		"Grégory Doche (French)",
		"Jarosław Pietras (Polish)",
		"Alejandro G. de la Muñoza (Spanish)",
		"Raymund Zacharias (German)",
		{type= "subsection", name= "5.0.10 update"},
		"Milène Gauthier-Sabourin (Arvaneth) (French)",
		{type= "subsection", name= "5.0.11 update"},
		"Joel Robert Justiawan (JOELwindows7) (Indonesian)",
	},
	{
		name= "Other Contributors",
		"Aldo Fregoso (Aldo_MX)", -- delays and much more. StepMania AMX creator
		"Alfred Sorenson", -- new lua bindings
		"A.C/@waiei", -- custom scoring fixes + Hybrid scoring
		"cerbo", -- lua bindings and other fun stuff
		"cesarmades", -- pump/cmd* noteskins
		"Chris Eldridge (kurisu)", -- dance-threepanel stepstype support
		"Christophe Goulet-LeBlanc (Kommisar)", -- songs
		"corec", -- various fixes
		"cybik", -- Android port
		"dbk2", -- mac builds, a couple actor behavior fixes, new lua bindings
		"djpohly", -- piuio kernel module, XML compatibility, other useful stuff
		"galopin", -- piu PlayStation2 usb mat support
		"gholms", -- automake 1.11 support
		"hanubeki (@803832)", -- beginner helper fix, among various other things
		"juanelote", -- SongManager:GetSongGroupByIndex, JumpToNext/PrevGroup logic mods
		"Kaox", -- pump/default noteskin
		-- Add Graphics/CreditsLogo name.png and change your entry to a table like this to look super pro.
		{logo= "kyzentun", name= "Kyzentun"}, -- new lua bindings, theme documentation
		"Lirodon", -- Lambda default theme on 5.1
		"Mad Matt", -- new lua bindings
		"Matt McCutchen", -- minor fix for some dance pads on linux
		"MrThatKid", -- nitg modifiers
		"NitroX72", -- pump/frame noteskin
		"nixtrix", -- various BMS features and other fixes
		"Petriform", -- default theme music
		"Prcuvu", -- various VS2015 related fixes
		"psmay", -- SextetStream driver and related things
		"桜為小鳩/Sakurana-Kobato (@sakuraponila)", -- custom scoring fixes
		"Samuel Kim (1a2a3a2a1a)", -- various beat mode fixes
		"tuxdude", -- minor changes to service menu layout
		"v1toko", -- x-mode from StepNXA
		"Wallacoloo", -- delete songs, other fixes
	},
	{
		name= "Special Thanks",
		"A Pseudonymous Coder", -- support
		"Bill Shillito (DM Ashura)", -- Music (not yet though)
		"cpubasic13", -- testing (a lot)
		"Dreamwoods",
		"Jason Bolt (LightningXCE)",
		"Jousway", -- Noteskins
		"Luizsan", -- creator of Delta theme
		"Matt1360", -- Automake magic + oitg bro
		"Renard",
		"Ryan McKanna (Plaguefox)",
		"Sta Kousin", --help with Japanese bug reports
	},
	{
		name= "Shoutouts",
		"The Lua team", -- lua project lead or some shit. super nerdy but oh hell.
		{logo= "mojang", name= "Mojang AB"}, -- minecraft forever -freem
		"Wolfire Games", -- piles of inspiration
		"NAKET Coder",
		"Ciera Boyd", -- you bet your ass I'm putting my girlfriend in the credits -shakesoda
		"#KBO",
		"Celestia Radio", -- LOVE AND TOLERANCE
		"Perkedel Corporation", -- Joel's company.
		"You showed us... your ultimate dance",
	},
}

local kyzentuns_fancy_value= 16

local special_logos= {
	kyzentun= Def.ActorMultiVertex{
		Name= "logo",
		Texture= THEME:GetPathG("CreditsLogo", "kyzentun"),
		OnCommand= function(self)
			self:SetDrawState{Mode= "DrawMode_Quads"}
			kyzentuns_fancy_value= math.random(2, 32)
			self:playcommand("fancy", {state= 0})
			self:queuecommand("normal_state")
		end,
		fancyCommand= function(self, params)
			local verts= {}
			local rlh= line_height - 2
			local sx= rlh * -1
			local sy= rlh * -.5
			local sp= rlh / kyzentuns_fancy_value
			local spt= 1 / kyzentuns_fancy_value
			local c= color("#ffffff")
			for x= 1, kyzentuns_fancy_value do
				local lx= sx + (sp * (x-1))
				local rx= sx + (sp * x)
				local ltx= spt * (x-1)
				local rtx= spt * x
				for y= 1, kyzentuns_fancy_value do
					local ty= sy + (sp * (y-1))
					local by= sy + (sp * y)
					local tty= spt * (y-1)
					local bty= spt * y
					if params.state == 1 then
						ltx= 0
						rtx= 1
						tty= 0
						bty= 1
					end
					verts[#verts+1]= {{lx, ty, 0}, {ltx, tty}, c}
					verts[#verts+1]= {{rx, ty, 0}, {rtx, tty}, c}
					verts[#verts+1]= {{rx, by, 0}, {rtx, bty}, c}
					verts[#verts+1]= {{lx, by, 0}, {ltx, bty}, c}
				end
			end
			self:SetVertices(verts)
		end,
		normal_stateCommand= function(self)
			self:linear(1)
			self:playcommand("fancy", {state= 0})
			self:queuecommand("split_state")
		end,
		split_stateCommand= function(self)
			self:linear(1)
			self:playcommand("fancy", {state= 1})
			self:queuecommand("normal_state")
		end,
	},
	mojang= Def.Actor{
		Name= "logo",
		OnCommand= function(self)
			self:GetParent():GetChild("name"):distort(.25) -- minecraft is broken, -kyz
		end
	},
}

-- Go through the credits and swap in the special logos.
for section in ivalues(stepmania_credits) do
	for entry in ivalues(section) do
		if type(entry) == "table" and special_logos[entry.logo] then
			entry.logo= special_logos[entry.logo]
		end
	end
end

local function position_logo(self)
	local name= self:GetParent():GetChild("name")
	local name_width= name:GetZoomedWidth()
	local logo_width= self:GetZoomedWidth()
	self:x(0 - (name_width / 2) - 4 - (logo_width / 2))
end

StepManiaCredits= {
	AddSection= function(section, pos, insert_before)
		if not section.name then
			lua.ReportScriptError("A section being added to the credits must have a name field.")
			return
		end
		if #section < 1 then
			lua.ReportScriptError("Adding a blank section to the credits doesn't make sense.")
			return
		end
		if type(pos) == "string" then
			for i, section in ipairs(stepmania_credits) do
				if section.name == pos then
					pos= i -- insert_after is default behavior
				end
			end
		end
		if pos and type(pos) ~= "number" then
			lua.ReportScriptError("Credits section '" .. tostring(pos) .. " not found, cannot use position to add new section.")
			return
		end
		pos= pos or #stepmania_credits
		if insert_before then
			pos= pos - 1
		end
		-- table.insert does funny things if you pass an index <= 0
		if pos < 1 then
			lua.ReportScriptError("Cannot add credits section at position " .. tostring(pos) .. ".")
			return
		end
		table.insert(stepmania_credits, pos, section)
	end,
	AddLineToScroller= function(scroller, text, command)
		if type(scroller) ~= "table" then
			lua.ReportScriptError("scroller passed to AddLineToScroller must be an actor table.")
			return
		end
		local actor_to_insert
		if type(text) == "string" or not text then
			actor_to_insert= Def.ActorFrame{
				Def.BitmapText{
					Font= "Common Normal",
					Text = text or "";
					OnCommand = command or lineOn;
				}
			}
		elseif type(text) == "table" then
			actor_to_insert= Def.ActorFrame{
				Def.BitmapText{
					Name= "name", Font= "Common Normal",
					Text = text.name or "",
					InitCommand = command or lineOn,
				},
			}
			if text.logo then
				if type(text.logo) == "string" then
					actor_to_insert[#actor_to_insert+1]= Def.Sprite{
						Name= "logo",
						InitCommand= function(self)
							-- Use LoadBanner to disable the odd dimension warning.
							self:LoadBanner(THEME:GetPathG("CreditsLogo", text.logo))
							-- Scale to slightly less than the line height for padding.
							local yscale= (line_height-2) / self:GetHeight()
							self:zoom(yscale)
							-- Position logo to the left of the name.
							position_logo(self)
						end
					}
				else -- assume logo is an actor
					-- Insert positioning InitCommand.
					text.logo.InitCommand= position_logo
					actor_to_insert[#actor_to_insert+1]= text.logo
				end
			end
		end
		table.insert(scroller, actor_to_insert)
	end,
	Get= function()
		-- Copy the base credits and add the copyright message at the end.
		local ret= DeepCopy(stepmania_credits)
		ret[#ret+1]= StepManiaCredits.RandomCopyrightMessage()
		return ret
	end,
	RandomCopyrightMessage= function()
		return {
			name= "Copyright",
			"StepMania is released under the terms of the MIT license.",
			"If you paid for the program you've been " .. Fooled() .. ".",
			"All content is the sole property of their respectful owners."
		}
	end,
	SetLineHeight= function(height)
		if type(height) ~= "number" then
			lua.ReportScriptError("height passed to StepManiaCredits.SetLineHeight must be a number.")
			return
		end
		line_height= height
	end
}
