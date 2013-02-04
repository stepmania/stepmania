-- UserPreferences: User Preferences "Module"
-- Written by AJ Kelly of KKI Labs / Version 2.11-ssc
-- (modified slightly for Cerulean Skies 2's disregard of the name of the
-- themeinfo variable lol :p)

--[[
the first released version was broken.
this version aims to be simpler, and therefore work.

[changelog]

v2.11-ssc
	Remove EnvUtils references; we have it in sm-ssc.

v2.1-ssc
	sm-ssc version of UserPrefs. We can now assume players have certain
	functionality, like RageFile:GetError().

v2.1
	Added type specific GetUserPref functions.

[usage]
First, edit PrefPath to match your theme.
If you use ThemeInfo, then you shouldn't have to edit this.

If you're not using ThemeInfo, then you can replace

".. themeInfo.Name .." with the theme's folder name.

ThemeInfo is documented at http://kki.ajworld.net/wiki/ThemeInfo.lua

After that's set up, read the docs.
]]
local PrefPath = "Save/UserPrefs/".. THEME:GetThemeDisplayName() .."/"

--[[ begin internal stuff; no need to edit below this line. ]]

-- Local internal function to write envs. ___Not for themer use.___
local function WriteEnv(envName,envValue)
	return setenv(envName,envValue)
end

function ReadPrefFromFile(name)
	local f = RageFileUtil.CreateRageFile()
	local fullFilename = PrefPath..name..".cfg"
	local option

	if f:Open(fullFilename,1) then
		option = tostring( f:Read() )
		WriteEnv(name,option)
		f:destroy()
		return option
	else
		local fError = f:GetError()
		Trace( "[FileUtils] Error reading ".. fullFilename ..": ".. fError )
		f:ClearError()
		f:destroy()
		return nil
	end
end

function WritePrefToFile(name,value)
	local f = RageFileUtil.CreateRageFile()
	local fullFilename = PrefPath..name..".cfg"

	if f:Open(fullFilename, 2) then
		f:Write( tostring(value) )
		WriteEnv(name,value)
	else
		local fError = f:GetError()
		Trace( "[FileUtils] Error writing to ".. fullFilename ..": ".. fError )
		f:ClearError()
		f:destroy()
		return false
	end

	f:destroy()
	return true
end

--[[ end internal functions; still don't edit below this line ]]

function GetUserPref(name)
	return ReadPrefFromFile(name)
end

function SetUserPref(name,value)
	return WritePrefToFile(name,value)
end

--[[ type specific, for when you want to be lazy ]]
-- XXX: make set funcs, since I hate dealing with colors and I know
-- other themers would too.

-- GetUserPrefB: boolean
function GetUserPrefB(name)
	-- this one is a bit trickier.
	local pref = ReadPrefFromFile(name)

	if type(pref) == "string" then
		pref = string.lower(pref)
		if pref == "true" or cmp == "t" then
			return true
		elseif pref == "false" or cmp == "f" then
			return false
		else
			Trace("Error in GetUserPrefB(".. name ..") converting from string" )
			return false
		end
	elseif type(pref) == "number" then
		-- both 0 and -1 are false; if you want to change this, 
		-- feel free to remove "or pref == -1".
		if pref == 0 or pref == -1 then

		else
			return true
		end
	end
end

-- GetUserPrefC: color
function GetUserPrefC(name)
	-- XXX: make sure it's grabbing a string that can be turned into a color
	-- and also possibly handle HSV values too.
	return color( ReadPrefFromFile(name) )
end

-- GetUserPrefN: numbers (integers, floats)
function GetUserPrefN(name)
	return tonumber( ReadPrefFromFile(name) )
end

--[[
Copyright © 2008-2009 AJ Kelly/KKI Labs
All rights reserved.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
]]
