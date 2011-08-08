-- used for LightsStateToHex
local lightsToHex =
{
	CabinetLight_MarqueeUpLeft	= 0x100000,
	CabinetLight_MarqueeUpRight	= 0x010000,
	CabinetLight_MarqueeLrLeft	= 0x001000,
	CabinetLight_MarqueeLrRight	= 0x000100,
	CabinetLight_BassLeft		= 0x000010,
	CabinetLight_BassRight		= 0x000001
}

return
{
	Name = "Print",
	Desc = "Prints lights data to standard output",
	ModuleType = "Lights",

	Init = function(self)
		Trace( "LightsModule_Print ("..tostring(self)..") initting now" )
		return true
	end,

	Exit = function(self)
		Trace( "LightsModule_Print: exiting now" )
	end,

	Iter = 0,

	Update = function(self, ls)
		self.Iter = self.Iter + 1
		local hex = self:LightsStateToHex(ls)
		if hex then
			Trace( ("%06x (%d iterations)"):format(hex, self.Iter) )
			self.Iter = 0
		end
	end,

	LastOutput = 0,

	LightsStateToHex = function( self, ls )
		local ret = 0

		for light, hex in pairs(lightsToHex) do
			if ls[light] then ret = bit.bor(ret, hex) end
		end

		-- don't output redundant data
		if ret == LastOutput then return end

		LastOutput = ret
		return ret
	end,
}
