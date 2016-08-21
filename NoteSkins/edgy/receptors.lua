-- White is the target color for the beat update command.
local colorWhite = {1, 1, 1, 1}
-- The column width to use assuming single style.
local columnWidth = 48
-- An initialization function for the receptors.
local function zoomTap(note)
  note:zoom(columnWidth / 64)
end

return function(button_list, stepstype, skin_parameters)
  local rotations = {DownLeft = 270, UpLeft = 0, Center = 0, UpRight = 90, DownRight = 180}
  local targets = {
    DownLeft = "corner",
    UpLeft = "corner",
    Center = "center",
    UpRight = "corner",
    DownRight = "corner"
  }
  -- What to return
  local ret = {}
  for i, button in ipairs(button_list) do
    local texture = "receptor-" .. targets[button] .. ".png"
    ret[i] = Def.ActorFrame {
      InitCommand = function(self)
        self:zoom(columnWidth / 64):rotationz(rotations[button] or 0):effectclock("beat")
          :draworder(notefield_draw_order.receptor)
      end,
			WidthSetCommand= function(self, param)
				param.column:set_layer_fade_type(self, "FieldLayerFadeType_Receptor")
			end,
      Def.Sprite {
        Texture = texture,
        -- Do not give a sneak peak of what to hit for classic style gameplay.
        BeatUpdateCommand = function(self, param)
          self:glow{1, 1, 1, (1 - param.beat * 2) / 4}
          self:diffuse(colorWhite)
          if param.pressed then
            self:zoom(.75)
          elseif param.lifted then
            self:zoom(1)
          end
        end
      }
    }
  end
  return ret
end
