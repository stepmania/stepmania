-- Alright, this is how we do things around here:
-- Write a table
local transition_params = {
	color = color("#000000"),
	transition_type = "in"
}

-- Give it to another worker and let them do all the work with it
return LoadActor(THEME:GetPathB("", "_transition"), transition_params)

-- Job done.
