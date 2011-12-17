-- ProductivityHelpers: A set of useful aliases for theming.
--[[ 3.9 Functions ]]
Game = {
	GetStage = function()
		
	end,
}

--[[ ----------------------------------------------------------------------- ]]

--[[ helper functions ]]
function pname(pn) return ToEnumShortString(pn) end

function ThemeManager:GetAbsolutePath(sPath)
  sFinPath = "/Themes/"..self:GetCurThemeName().."/"..sPath
  assert(RageFileManager.DoesFileExist(sFinPath), "the theme element "..sPath.." is missing")
  return sFinPath
end

--[[ end helper functions ]]
-- this code is in the public domain.
