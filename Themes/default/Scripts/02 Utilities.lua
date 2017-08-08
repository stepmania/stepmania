-- Placed here to keep shit from breaking until I get myself together long enough to deal with it all
function scale_to_fit(actor, width, height)
	local xscale= width / actor:GetWidth()
	local yscale= height / actor:GetHeight()
	actor:zoom(math.min(xscale, yscale))
end