return
{
	Name = "Pulse",
	ModuleType = "Input",

	NumUpdates = 0,

	-- once the update number reaches this count,
	-- DeviceButton_B1 is pulsed and the count is reset
	InsertOnUpdate = 1000,

	Init = function(self)
		return true
	end,

	PulseButton = function(self, button)
		self:ButtonPressed( button, 1 )
		self:ButtonPressed( button, 0 )
	end,

	Update = function(self)
		self.NumUpdates = self.NumUpdates + 1

		if SCREENMAN then
			SCREENMAN:SystemMessageNoAnimate( "self.NumUpdates = " .. tostring(self.NumUpdates) )
		end

		if self.NumUpdates == self.InsertOnUpdate then
			self:PulseButton( "DeviceButton_B1" )
			self.NumUpdates = 0
		end
	end,

	Exit = function(self) end
}
