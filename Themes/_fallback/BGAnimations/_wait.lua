local waitTime = ...

-- default to 1 second if parameter is missing
if not waitTime then waitTime = 1 end

return Def.Actor{ OnCommand=cmd(sleep,waitTime); };