function JudgmentTransformSharedCommand( self, params )
	local x = 0
	local y = 30
	if params.bReverse then y = y * -1 end

	if params.bCentered then
		if params.bReverse then
			y = y - 30
		else
			y = y + 40
		end
	end
	self:x( 0 )
	self:y( 0 )
end

function ComboTransformCommand( self, params )
	local x = 0
	local y = 30
	if params.bReverse then y = y * -1 end

	if params.bCentered then
		if params.bReverse then
			y = y - 30
		else
			y = y + 40
		end
	end
	self:x( 0 )
	self:y( 0 )
end
