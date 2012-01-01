--[[ People sometimes have problems where they rebind the enter key and thus
cannot press start. To work around this problem, this script deletes every key
binding to Enter. --]]

function ClearEnterMappedKeys()
	-- load Keymaps.ini
	local keymaps = IniFile.ReadFile("/Save/Keymaps.ini")

	-- iterate through the games
	for k,v in pairs(keymaps) do
		-- iterate through the potential key mappings
	 for l,w in pairs(v) do
			-- check for enter
			w:gsub("Key_enter","")
			-- write back
			keymaps[k][l] = w
		end
	end

	-- write Keymaps.ini back out. yay.
	IniFile.WriteFile("/Save/Keymaps.ini",keymaps)
end

-- actually run it.
ClearEnterMappedKeys()