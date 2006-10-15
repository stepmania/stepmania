-- Override Lua's loadfile to use lua.ReadFile.
function loadfile(file)
	local data, err = lua.ReadFile(file);
	if not data then
		return nil, ("what " .. file)
	end

	local chunk, err = load(
		function()
			local ret = data
			data = nil
			return ret
		end,
		"@" .. file );
	if not chunk then return nil, err end

	-- Set the environment, like loadfile does.
	setfenv( chunk, getfenv(2) );
	return chunk
end

-- Override Lua's dofile to use our loadfile.
function dofile(file)
	if not file then
		error( "dofile(nil) unsupported", 2 );
	end

	local chunk, err = loadfile(file)
	if not chunk then
		error( err, 2 );
	end

	return chunk
end

