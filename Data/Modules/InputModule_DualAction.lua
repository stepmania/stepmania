local Handle = nil;
local VendorID = 0x046D;
local ProductID = 0xC216;

return
{
	Desc = "Logitech DualAction",

	Init = function(self)
		Handle = LuaAPIHandle.Create( "USB" )
		if not Handle then
			return Warn( "Failed to create USB handle" )
		end

		Trace( ("API version: %d.%d"):format(Handle:GetRevision()) )

		Trace( "USB Handle: " .. tostring(Handle) )
		if not Handle:Open(VendorID, ProductID) then
			return Warn( "Failed to open device!" )
		end

		-- only set the config if it isn't set already
		if Handle:GetConfiguration() ~= 1 then
			Handle:SetConfiguration( 1 )

			-- setting configuration failed
			if Handle:GetError() ~= 0 then
				return Warn( "set config: " .. Handle:GetErrorStr() )
			end
		end

		if not Handle:ClaimInterface(0) then
			return Warn( "Failed to set interface 0: " .. Handle:GetErrorStr() )
		end

		Trace( "InputModule_DualAction: load succeeded" )
		return true
	end,

	LastInputData = 0,

	Update = function(self)
		local transferred, data = Handle:InterruptTransfer( 0x81, nil, 10 )

		-- don't report unchanged data
		if transferred == 0 then return end

		-- build a 16-bit value out of bytes 5 and 6
		local b1, b2 = data:byte(5, 6)
		local InputData = bit.bor( bit.lshift(b1, 8), b2 )

		SCREENMAN:SystemMessageNoAnimate( ("DualAction: %02x %02x"):format(b1, b2) )

		for i = 1, 16 do
			local mask = bit.lshift( 1, 16-i )
			local level = bit.band(InputData,mask) == 0 and 0 or 1
			self:ButtonPressed( "DeviceButton_B" .. tostring(i), level )
		end

		self.LastInputData = InputData
	end,

	Exit = function(self)
		if not Handle then return end

		Handle:Close()
		Handle:Destroy()
	end,
}
