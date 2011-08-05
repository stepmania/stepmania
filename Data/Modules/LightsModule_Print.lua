-- used for LightsStateToString
local lights =
{
	"CabinetLight_MarqueeUpLeft",
	"CabinetLight_MarqueeUpRight",
	"CabinetLight_MarqueeLrLeft",
	"CabinetLight_MarqueeLrRight",
	"CabinetLight_BassLeft",
	"CabinetLight_BassRight",
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

	Update = function(self, ls)
		self.Iter = self.Iter + 1
		local str = self:LightsStateToString(ls)
		if str then
			Trace( ("%s (%d iterations)"):format(str, self.Iter) )
			self.Iter = 0
		end
	end,


	LastOutput = "000000",

	Iter = 0,

	LightsStateToString = function( self, ls )
		local ret = ""

		-- build a string of 1s/0s describing the lights
		for i = 1, #lights do
			ret = ret .. ( ls[lights[i]] and "1" or "0" )
--			if ls[lights[i]] then ret = ret .. "1" else ret = ret .. "0" end
		end

		-- don't output redundant text
		if ret == LastOutput then return end

		LastOutput = ret
		return ret
	end,
}
