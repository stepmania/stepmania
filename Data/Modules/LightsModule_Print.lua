-- used for LightsStateToHex
local CabinetLightToHex =
{
	CabinetLight_MarqueeUpLeft	= 0x100000,
	CabinetLight_MarqueeUpRight	= 0x010000,
	CabinetLight_MarqueeLrLeft	= 0x001000,
	CabinetLight_MarqueeLrRight	= 0x000100,
	CabinetLight_BassLeft		= 0x000010,
	CabinetLight_BassRight		= 0x000001
}

local GameButtonToHex =
{
	-- Left, Right, Up, Down
	dance =
	{
		GameButton_Custom01	= 0x1000,
		GameButton_Custom02	= 0x0001,
		GameButton_Custom03	= 0x0010,
		GameButton_Custom04	= 0x0100,
	},

	-- UL, UR, Center, DL, DR
	pump =
	{
		GameButton_Custom01	= 0x10000,
		GameButton_Custom02	= 0x01000,
		GameButton_Custom03	= 0x00100,
		GameButton_Custom04	= 0x00010,
		GameButton_Custom05	= 0x00001,
	},
}

local GameControllers =
{
	"GameController_1",
	"GameController_2"
}

local function PrintTable( tbl )
	for k, v in pairs(tbl) do
		
		if type(v) ~= "table" then
			Trace( ("[%s] => %s"):format(tostring(k), tostring(v)) )
		else
			Trace( ("[%s] => {"):format(tostring(k)) )
			PrintTable( v )
			Trace( "}" )
		end
	end
end

return
{
	Desc = "Prints lights data to standard output",

	Init = function(self)
		Trace( "LightsModule_Print ("..tostring(self)..") initting now" )
		return true
	end,

	Exit = function(self)
		Trace( "LightsModule_Print: exiting now" )
	end,

	IsFirstUpdate = true,

	Update = function(self, ls)
		-- only do this once
		if self.IsFirstUpdate then
			PrintTable(ls)
			self.IsFirstUpdate = false
		end

		local cl, gc1, gc2 = self:LightsStateToHex(ls)

		local text = nil

		if gc1 and gc2 then
			text = ("%06x | P1: %05x | P2: %05x"):format(cl, gc1, gc2)
		else
			text = ("%06x"):format(cl)
		end

		SCREENMAN:SystemMessageNoAnimate( text )
	end,

	-- returns 3 hex values (cl, gc1, gc2) or 1 hex value (cl)
	LightsStateToHex = function( self, ls )
		local cabinet = 0

		for light, hex in pairs(CabinetLightToHex) do
			if ls[light] then cabinet = bit.bor(cabinet, hex) end
		end

		local buttons = { 0, 0 }

		-- this can happen if INPUTMAPPER does not exist
		for _, gc in ipairs(GameControllers) do
			if not ls[gc] then return cabinet end
		end

		local game = GAMESTATE:GetCurrentGame():GetName()
		local buttonsToHex = GameButtonToHex[game]
		if not buttonsToHex then return cabinet end -- unsupported game

		for i, gc in ipairs(GameControllers) do
			for button, hex in pairs(buttonsToHex) do
				if ls[gc][button] then
					buttons[i] = bit.bor(buttons[i], hex)
				end
			end
		end

		return cabinet, buttons[1], buttons[2]
	end,
}
