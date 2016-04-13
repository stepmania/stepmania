--[[
FontLoader by NAKET Coder

how it works:

Say you have a new generation Fonts folder, which is something like...
\Fonts
 \frutiger
  \_32pt
 \scooter
  \_36px numbers
  \_32px metal


in your bganims, call
Font(family, style)

where:
family = font family [e.g. "frutiger", "scooter"]
style = font style [e.g. "32pt", "24px shiny"]
--]]

function Font(family,style) return LoadFont( "", family.."/_"..style ) end

function FontPath(family,style) return THEME:GetPathF("",family.."/_"..style) end
