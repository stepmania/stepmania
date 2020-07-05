-- A shim to make an honest-to-goodness lua table (array) out of the
-- userdata DisplaySpecs we get from c++
local cachedSpecs = nil

local function GetDisplaySpecs()
	if cachedSpecs == nil then
		local specs = DISPLAY:GetDisplaySpecs()
		t = {
			ById= function(self, id)
				for i, d in ipairs(self) do
					if d:GetId() == id or
					((id == '' or id == nil) and d:IsVirtual()) then
						return self[i]
					end
				end
				-- default to returning first DisplaySpec
				return self[1]
			end,
			-- hold on to a reference to the userdata, so it isn't GC'd prematurely
			c_specs= specs
		}
		for i = 1, #specs do
			t[i] = specs[i]
		end

		cachedSpecs = t
	end

	return cachedSpecs
end

-- Ease lookup of OptionRow effects masks
local OE = OptEffect:Reverse()

function ConfDisplayMode()
	local effectsMask = 2^OE["OptEffect_SavePreferences"]
	effectsMask = effectsMask + 2^OE["OptEffect_ApplyGraphics"]

	local specs = GetDisplaySpecs()
	local choices = {}
	for i, d in ipairs(specs) do
		choices[i] = d:GetName()
	end
	local origDisplay = PREFSMAN:GetPreference("DisplayId")
	local origWindowed = PREFSMAN:GetPreference("Windowed")
	choices[#choices + 1] = "Windowed"

	local t = {
		Name= "Windowed",
		GoToFirstOnStart= false,
		OneChoiceForAllPlayers= true,
		ExportOnChange= false,
		LayoutType= "ShowAllInRow",
		SelectType= "SelectOne",

		Choices= choices,
		LoadSelections= function(self, list, pn)
			local windowed = PREFSMAN:GetPreference("Windowed")
			local displayIdx = FindValue(specs, specs:ById(PREFSMAN:GetPreference("DisplayId")))
			local i = windowed and #self.Choices or displayIdx
			list[i] = true
		end,
		SaveSelections= function(self, list, pn)
			local choice = FindSelection(list)
			local selWindowed = choice == #self.Choices
			local selDisplay = selWindowed and origDisplay or specs[choice]:GetId()

			-- If we've changed from the established preferences in place at our
			-- instantiation, then we'll return our effects
			if selWindowed ~= origWindowed
			or selDisplay ~= origDisplay then
				return effectsMask
			else
				return 0
			end
		end,
		NotifyOfSelection= function(self, pn, choice)
			local curDisplay = PREFSMAN:GetPreference("DisplayId")
			local curWindowed = PREFSMAN:GetPreference("Windowed")
			local selWindowed = choice == #self.Choices
			local selDisplay = selWindowed and curDisplay or specs[choice]:GetId()
			-- If we've changed from what's currently saved in PREFSMAN,
			-- then we'll broadcast that state
			if selWindowed ~= curWindowed
			or selDisplay ~= curDisplay then
				PREFSMAN:SetPreference("Windowed", selWindowed)
				PREFSMAN:SetPreference("DisplayId", selDisplay)
				MESSAGEMAN:Broadcast("DisplayChoiceChanged")
			end
		end
	}
	return t
end

-- TODO: here, I'm replicating the AspectRatios table from "02 Utilities.lua",
-- but in a friendly format
-- name can optionally be provided as a user-facing string for ConfAspectRatio
local winFracs = {
	{n=3,  d=4},
	{n=1,  d=1},
	{n=5,  d=4},
	{n=4,  d=3},
	{n=16, d=10},
	{n=16, d=9},
	{n=8,  d=3},
	{n=64, d=27, name="21:9"},
	{n=9,  d=16},
}
-- Maximum distance between aspect ratios to be considered equivalent
-- (approximately half the distance between 16:9 and 16:10)
local RATIO_EPSILON = .044


local function GetWindowAspectRatios()
	local ratios = {}
	for i, f in ipairs(winFracs) do
		ratios[math.round(f.n/f.d, 3)] = f
	end

	return ratios
end

-- build a map from DisplayId to supported aspect ratios
local function GetDisplayAspectRatios(specs)
	local ratios = {}
	local recognized = GetWindowAspectRatios()
	for _, d in ipairs(specs) do
		local dRatios = {}
		for j, mode in ipairs(d:GetSupportedModes()) do
			local trueRatio = mode:GetWidth()/mode:GetHeight()
			local truncRatio = math.round(trueRatio, 3)
			if not dRatios[truncRatio] then
				local f = {}
				-- Check if this is a known aspect ratio, or practically the
				-- same (e.g. 1366x768 is typically considered 16:9)
				for knownRatio, fraction in pairs(recognized) do
					if math.abs(trueRatio - (fraction.n/fraction.d)) < RATIO_EPSILON then
						truncRatio = knownRatio
						f = fraction
						break
					end
				end

				-- If we didn't find a recognized ratio, then register a new ratio
				-- as the reduced fraction of width / height
				if not f.n then
					local w = mode:GetWidth()
					local h = mode:GetHeight()
					local gcd = math.gcd(w, h)
					dRatios[trueRatio] = {n=w/gcd, d=h/gcd}
				else
					dRatios[truncRatio] = f
				end
			end
		end
		ratios[d:GetId()] = dRatios
		-- Allow looking up the virtual display's aspect ratio by empty string
		if d:IsVirtual() then
			ratios[""] = dRatios
		end
	end

	return ratios
end

-- hack, use as a space to share value between ConfAspectRatio() and
-- ConfDisplayResoltuion(). Can't use PREFSMAN:SetPreference("DisplayAspectRatio", x)
-- because that change take effect *immediately* (it is read in the rendering loop)
local CurAspectRatio = 0

function ConfAspectRatio()
	local effectsMask = 2^OE["OptEffect_SavePreferences"]
	effectsMask = effectsMask + 2^OE["OptEffect_ApplyGraphics"]
	effectsMask = effectsMask + 2^OE["OptEffect_ApplyAspectRatio"]
	local specs = GetDisplaySpecs()
	local origAspectRatio = PREFSMAN:GetPreference("DisplayAspectRatio")
	CurAspectRatio = origAspectRatio

	local dRatios = GetDisplayAspectRatios(specs)
	local wRatios = GetWindowAspectRatios()
	local t = {
		Name= "DisplayAspectRatio",
		GoToFirstOnStart= false,
		OneChoiceForAllPlayers= true,
		ExportOnChange= false,
		LayoutType= "ShowAllInRow",
		SelectType= "SelectOne",
		ReloadRowMessages= {"DisplayChoiceChanged"},
		LoadSelections= function(self, list, pn)
			local r = CurAspectRatio
			for i, v in ipairs(self.ChoiceVals) do
				if math.abs(CurAspectRatio - v) < RATIO_EPSILON then
					list[i] = true
					break
				end
			end
		end,

		SaveSelections= function(self, list, pn)
			local i = FindSelection(list)
			PREFSMAN:SetPreference("DisplayAspectRatio", self.ChoiceVals[i])
			if math.abs(self.ChoiceVals[i] - origAspectRatio) < RATIO_EPSILON then
				return 0
			else
				return effectsMask
			end
		end,

		NotifyOfSelection= function(self, pn, choice)
			local selRatio = self.ChoiceVals[choice]
			if math.abs(selRatio-CurAspectRatio) >= RATIO_EPSILON then
				CurAspectRatio = selRatio
				MESSAGEMAN:Broadcast("AspectRatioChoiceChanged")
			end
		end,

		Reload= function(self)
			local origVals = self.ChoiceVals
			self:GenChoices()

			-- Pass along the message to ConfDisplayResolution()
			-- regardless, but important we do this *after* self:GenChoices()
			-- to ensure we've updated the CurAspectRatio in accordance
			-- with what this display supports
			MESSAGEMAN:Broadcast("AspectRatioChoiceChanged")

			if #origVals ~= #self.ChoiceVals then
				return "ReloadChanged_All"
			end

			for i, v in ipairs(origVals) do
				if origVals[i] ~= self.ChoiceVals[i] then
					return "ReloadChanged_All"
				end
			end

			return "ReloadChanged_None"
		end,

		GenChoices= function(self)
			-- Determine the available aspect ratios for the current display mode
			local isWindowed = PREFSMAN:GetPreference("Windowed")
			local curDisplayId = PREFSMAN:GetPreference("DisplayId")
			local ratios = isWindowed and wRatios or (dRatios[curDisplayId] or dRatios[specs[1]:GetId()])
			local choices = {}
			local vals = {}
			foreach_ordered(ratios, function(v, f)
						vals[#vals + 1] = f.n/f.d
						choices[#choices + 1] = (type(f.name)=="string" and f.name) or ("%d:%d"):format(f.n, f.d)
			end)
			self.Choices = choices
			self.ChoiceVals = vals

			-- Get the closest aspect ratio to the currently-configured one
			-- that this display mode supports
			local closestRatio = vals[1]
			local closestDist = math.abs(vals[1] - CurAspectRatio)
			for _, v in ipairs(vals) do
				local dist = math.abs(v - CurAspectRatio)
				if dist < closestDist then
					closestRatio = v
					closestDist = dist
				end
			end
			CurAspectRatio = closestRatio
		end
	}
	t:GenChoices()
	return t
end

local function GenerateFeasibleWindowSizesForRatio(specs, r)
	-- Get the largest feasible window size *for a single monitor*,
	-- then generate a sequence of smaller window sizes, until
	-- one of height/width < 300 (arbitrary)
	-- TODO: Get fancy and consider larger rectangles that span
	-- multiple monitors
	local maxWinWidth = 0
	for _, d in ipairs(specs) do
		local modes = d:GetSupportedModes()
		local dLargestMode = modes[#modes]
		local w = dLargestMode:GetWidth()
		local h = dLargestMode:GetHeight()
		local dMaxWinWidth = w/r <= h and w or r*h
		maxWinWidth = dMaxWinWidth > maxWinWidth and dMaxWinWidth or maxWinWidth
	end

	-- Generate successively smaller rectangles
	local sizes = {}

	local w = maxWinWidth
	local h = maxWinWidth/r
	while w > 300 and h > 300 do
		sizes[#sizes + 1] = {w=math.round(w), h=math.round(h)}
		w = 0.75*w
		h = w/r
	end

	-- Reverse the list (sort in increasing size)
	local rsizes = {}
	for i = #sizes, 1, -1 do
		rsizes[i] = sizes[#sizes - i + 1]
	end

	return rsizes
end

local function GetFeasibleWindowSizesForRatio(specs, r)
	-- First, try to see if we have a sufficiently large set of
	-- resolutions to pick from in the list(s) of supported modes for the display(s)
	-- (since those resolutions tend to be "natural" looking), otherwise just generate
	-- some window sizes (which might look weird).
	local widths = {}
	for _, d in ipairs(specs) do
		for _, m in ipairs(d:GetSupportedModes()) do
			local w = m:GetWidth()
			local h = m:GetHeight()
			if math.abs((w/h) - r) < RATIO_EPSILON then
				widths[w] = h
			end
		end
	end

	if table.itemcount(widths) < 4 then
		return GenerateFeasibleWindowSizesForRatio(specs, r)
	else
		local resolutions = {}
		for w, h in pairs(widths) do
			resolutions[#resolutions + 1] = {w=w, h=h}
		end
		table.sort(resolutions, function (a, b) return a.w < b.w end)
		return resolutions
	end

end


local function GetDisplayResolutionsForRatio(d, r)
	local sizes = {}
	for _, m in ipairs(d:GetSupportedModes()) do
		local w = m:GetWidth()
		local h = m:GetHeight()
		local trueRatio = w/h
		if math.abs(trueRatio - r) < RATIO_EPSILON then
			-- This depends on the fact that GetSupportedModes returns
			-- a *sorted* list
			if #sizes == 0 or sizes[#sizes].w ~= w or sizes[#sizes].h ~= h then
				sizes[#sizes + 1] = {w=m:GetWidth(), h=m:GetHeight()}
			end
		end
	end
	return sizes
end

function ConfDisplayResolution()
	local effectsMask = 2^OE["OptEffect_SavePreferences"]
	effectsMask = effectsMask + 2^OE["OptEffect_ApplyGraphics"]
	effectsMask = effectsMask + 2^OE["OptEffect_ApplyAspectRatio"]
	local specs = GetDisplaySpecs()
	local origWidth = PREFSMAN:GetPreference("DisplayWidth")
	local origHeight = PREFSMAN:GetPreference("DisplayHeight")

	local t = {
		Name= "DisplayResolution",
		GoToFirstOnStart= false,
		OneChoiceForAllPlayers= true,
		ExportOnChange= false,
		LayoutType= "ShowAllInRow",
		SelectType= "SelectOne",
		ReloadRowMessages= {"AspectRatioChoiceChanged"},
		LoadSelections= function(self, list, pn)
			local w = PREFSMAN:GetPreference("DisplayWidth")
			local h = PREFSMAN:GetPreference("DisplayHeight")
			for i, v in ipairs(self.ChoiceVals) do
				if v.w == w and v.h == h then
					list[i] = true
					break
				end
			end
		end,

		SaveSelections= function(self, list, pn)
			local i = FindSelection(list)
			local w = self.ChoiceVals[i].w
			local h = self.ChoiceVals[i].h
			PREFSMAN:SetPreference("DisplayWidth", w)
			PREFSMAN:SetPreference("DisplayHeight", h)
			if w == origWidth and h == origHeight then
				return 0
			else
				return effectsMask
			end
		end,

		NotifyOfSelection= function(self, pn, choice)
			local selRes = self.ChoiceVals[choice]
			local curWidth = PREFSMAN:GetPreference("DisplayWidth")
			local curHeight = PREFSMAN:GetPreference("DisplayHeight")
			if selRes.w ~= curWidth or selRes.h ~= curHeight then
				PREFSMAN:SetPreference("DisplayWidth", selRes.w)
				PREFSMAN:SetPreference("DisplayHeight", selRes.h)
				MESSAGEMAN:Broadcast("DisplayResolutionChoiceChanged")
			end
		end,

		Reload= function(self)
			local origVals = self.ChoiceVals
			self:GenChoices()

			-- Pass along the message to ConfRefreshRate() regardless of
			-- what happens here. Do this *after* GenChoices() so that
			-- *if* currently configured resolution needed to change in response
			-- to change in configured AspectRatio, ConfRefreshRate() can change
			-- accordingly
			MESSAGEMAN:Broadcast("DisplayResolutionChoiceChanged")

			if #origVals ~= #self.ChoiceVals then
				return "ReloadChanged_All"
			end

			for i, v in ipairs(origVals) do
				if origVals[i] ~= self.ChoiceVals[i] then
					return "ReloadChanged_All"
				end
			end

			return "ReloadChanged_None"
		end,

		GenChoices= function(self)
			local isWindowed = PREFSMAN:GetPreference("Windowed")
			local curDisplay = specs:ById(PREFSMAN:GetPreference("DisplayId"))
			local curRatio = CurAspectRatio ~= 0 and CurAspectRatio or PREFSMAN:GetPreference("DisplayAspectRatio")
			local resolutions = isWindowed and GetFeasibleWindowSizesForRatio(specs, curRatio)
				or GetDisplayResolutionsForRatio(curDisplay, curRatio)

			local choices = {}
			local vals = {}
			for i, r in ipairs(resolutions) do
				vals[#vals + 1] = r
				choices[#choices + 1] = tostring(r.w) .. 'x' .. tostring(r.h)
			end
			self.Choices = choices
			self.ChoiceVals = vals

			-- If the currently configured resolution isn't available at this
			-- aspect ratio, then pick the closest resolution
			local w = PREFSMAN:GetPreference("DisplayWidth")
			local h = PREFSMAN:GetPreference("DisplayHeight")
			local closest = nil
			local mindist = -1
			for i, v in ipairs(vals) do
				local dist = math.sqrt((v.w - w)^2 + (v.h - h)^2)
				if mindist == -1 or dist < mindist then
					mindist = dist
					closest = v
				end
			end
			PREFSMAN:SetPreference("DisplayWidth", closest.w)
			PREFSMAN:SetPreference("DisplayHeight", closest.h)
		end
	}
	t:GenChoices()
	return t
end

function GetDisplayRatesForResolution(d, w, h)
	local rates = {}
	for _, m in ipairs(d:GetSupportedModes()) do
		if m:GetWidth() == w and m:GetHeight() == h then
			rates[#rates + 1] = math.round(m:GetRefreshRate())
		end
	end
	return rates
end

function ConfRefreshRate()
	local effectsMask = 2^OE["OptEffect_SavePreferences"]
	effectsMask = effectsMask + 2^OE["OptEffect_ApplyGraphics"]
	local specs = GetDisplaySpecs()
	local origRate = PREFSMAN:GetPreference("RefreshRate")

	local t = {
		Name= "RefreshRate",
		GoToFirstOnStart= false,
		OneChoiceForAllPlayers= true,
		ExportOnChange= false,
		LayoutType= "ShowAllInRow",
		SelectType= "SelectOne",
		ReloadRowMessages= {"DisplayResolutionChoiceChanged"},
		LoadSelections= function(self, list, pn)
			local r = PREFSMAN:GetPreference("RefreshRate")
			local i = FindValue(self.ChoiceVals, r)
			list[i] = true
		end,

		SaveSelections= function(self, list, pn)
			local i = FindSelection(list)
			local r = self.ChoiceVals[i]
			PREFSMAN:SetPreference("RefreshRate", r)
			if r ~= origRate then
				return effectsMask
			else
				return 0
			end
		end,

		NotifyOfSelection= function(self, pn, choice)
			local selRate = self.ChoiceVals[choice]
			local curRate = PREFSMAN:GetPreference("RefreshRate")
			if selRate ~= curRate then
				PREFSMAN:SetPreference("RefreshRate", selRate)
			end
		end,

		Reload= function(self)
			local origVals = self.ChoiceVals
			self:GenChoices()
			if #origVals ~= #self.ChoiceVals then
				return "ReloadChanged_All"
			end

			for i, v in ipairs(origVals) do
				if origVals[i] ~= self.ChoiceVals[i] then
					return "ReloadChanged_All"
				end
			end

			return "ReloadChanged_None"
		end,

		GenChoices= function(self)
			local isWindowed = PREFSMAN:GetPreference("Windowed")
			local d = specs:ById(PREFSMAN:GetPreference("DisplayId"))
			local w = PREFSMAN:GetPreference("DisplayWidth")
			local h = PREFSMAN:GetPreference("DisplayHeight")
			local rates = isWindowed and {}
				or GetDisplayRatesForResolution(d, w, h)

			local choices = {"Default"}
			local choiceVals = {REFRESH_DEFAULT}
			for i, r in ipairs(rates) do
				choiceVals[#choiceVals + 1] = math.round(r)
				choices[#choices + 1] = tostring(math.round(r))
			end

			self.Choices = choices
			self.ChoiceVals = choiceVals

			-- If the currently configured refresh rate isn't available at this
			-- resolution, then pick the closest one (so long as it's within
			-- a fixed threshold)
			local curRate = PREFSMAN:GetPreference("RefreshRate")
			local threshold = 10
			local closestIdx = nil
			local mindist = -1
			for i, r in ipairs(choiceVals) do
				local dist = math.abs(r - curRate)
				if mindist == -1 or dist < mindist then
					mindist = dist
					closestIdx = i
				end
			end
			local curRateIdx = mindist < threshold and closestIdx or 1
			PREFSMAN:SetPreference("RefreshRate", choiceVals[curRateIdx])
		end
	}
	t:GenChoices()
	return t
end

function ConfFullscreenType()
	local effectsMask = 2^OE["OptEffect_SavePreferences"]
	effectsMask = effectsMask + 2^OE["OptEffect_ApplyGraphics"]

	local FULLSCREEN_EXCLUSIVE = 0
	local FULLSCREEN_BORDERLESS_WIN = 1

	local fsbwSupported = DISPLAY:SupportsFullscreenBorderlessWindow()

	local choiceVals = {FULLSCREEN_EXCLUSIVE}
	local choices = {"Default"}
	if fsbwSupported then
		choices = {"Exclusive", "Borderless Window"}
		choiceVals = {FULLSCREEN_EXCLUSIVE, FULLSCREEN_BORDERLESS_WIN}
	end
	local origFSType = PREFSMAN:GetPreference("FullscreenIsBorderlessWindow") and FULLSCREEN_BORDERLESS_WIN or FULLSCREEN_EXCLUSIVE

	local t = {
		Name= "FullscreenType",
		GoToFirstOnStart= false,
		OneChoiceForAllPlayers= true,
		ExportOnChange= false,
		LayoutType= "ShowAllInRow",
		SelectType= "SelectOne",
		Choices= choices,
		ChoiceVals= choiceVals,

		LoadSelections= function(self, list, pn)
			local fsType = PREFSMAN:GetPreference("FullscreenIsBorderlessWindow") and FULLSCREEN_BORDERLESS_WIN
				or FULLSCREEN_EXCLUSIVE
			local i = FindValue(self.ChoiceVals, fsType)
			list[i] = true
		end,
		SaveSelections= function(self, list, pn)
			local selFSType = self.ChoiceVals[FindSelection(list)]
			PREFSMAN:SetPreference("FullscreenIsBorderlessWindow", selFSType == FULLSCREEN_BORDERLESS_WIN)
			if selFSType ~= origFSType then
				return effectsMask
			else
				return 0
			end
		end
	}

	return t
end
