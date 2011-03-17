-- fileutils: quick and dirty, not user prefs oriented file io
File = {
	Write = function(path,buf)
		local f = RageFileUtil.CreateRageFile()
		if f:Open(path, 2) then
			f:Write( tostring(buf) )
			f:destroy()
			return true
		else
			Trace( "[FileUtils] Error writing to ".. path ..": ".. f:GetError() )
			f:ClearError()
			f:destroy()
			return false
		end
	end,
	Read = function(path)
		local f = RageFileUtil.CreateRageFile()
		local ret = ""
		if f:Open(path, 1) then
			ret = tostring( f:Read() )
			f:destroy()
			return ret
		else
			Trace( "[FileUtils] Error reading from ".. path ..": ".. f:GetError() )
			f:ClearError()
			f:destroy()
			return nil
		end
	end
}
-- this code if public domain and/or has no copyright, depending on your
-- country's laws. I wish for you to use this code freely, without restriction.