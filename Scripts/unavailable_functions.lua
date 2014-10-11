--This provides wrappers to prevent themes that used functions that are now unavailable from crashing.

Scoring={}

setmetatable(Scoring,{__index=function() return function() lua.ReportScriptError("Lua scoring unimplemented") end end})
