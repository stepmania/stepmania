-- Hide if b is true, but don't unhide if b is false.
function Actor:hide_if(b)
	if b then
		self:hidden(1)
	end
end

