-- return the function to load the actors needed.
return function(button_list, stepstype, skin_parameters)
  -- only the tap notes need to be rotated.
  local tapRotations = {DownLeft = 270, UpLeft = 0, Center = 0, UpRight = 90, DownRight = 180}
  -- Different tap buttons are needed depending on if it's center or not.
  local tapButtons = {
    DownLeft = "up-left",
    UpLeft = "up-left",
    Center = "center",
    UpRight = "up-left",
    DownRight = "up-left"
  }
  -- No hold/roll flips needed. Just focus on the button names.
  local holdButtons = {
    DownLeft = "down-left",
    UpLeft = "up-left",
    Center = "center",
    UpRight = "up-right",
    DownRight = "down-right"
  }

  -- The valid tap states procedurally generated.
  local quantizations = {4, 8, 12, 16, 24, 32, 48, 64}
  local tapStates = {}
  local holdStates = {}
  for i, quantization in ipairs(quantizations) do
    local tapStateRow = {}
    local holdStateRow = {}
    for j=1,8 do
      tapStateRow[j] = ((i - 1) * 8) + j
    end
    for j=1,2 do
      holdStateRow[j] = ((i - 1) * 4) + j
    end
    tapStates[quantization] = tapStateRow
    holdStates[quantization] = holdStateRow
  end

  -- Activate ITG Mode here.
  local isClassic = skin_parameters.colors.color_style == "Classic"
  if (not(isClassic) and skin_parameters.colors.rhythm_quarter_color == "Red") then
    tapStates[4], tapStates[8] = tapStates[8], tapStates[4]
    holdStates[4], holdStates[8] = holdStates[8], holdStates[4]
  end

  -- The state animations for mono-color non hold style notes.
  local classicTapStates = {
    DownLeft = tapStates[4],
    UpLeft = tapStates[8],
    Center = tapStates[16],
    UpRight = tapStates[8],
    DownRight = tapStates[4]
  }
  -- The state animations for mono-color hold style notes.
  local classicHoldStates = {
    DownLeft = holdStates[4],
    UpLeft = holdStates[8],
    Center = holdStates[16],
    UpRight = holdStates[8],
    DownRight = holdStates[4]
  }

  -- Currently, quantizations are based off of 48.
  local partsPerBeat = 48

  -- Preserve the original default length data for holds in case of changes.
  local holdLengthData = {
    start_note_offset = -.25,
    end_note_offset = .25,
    head_pixs = 32,
    body_pixs = 64,
    tail_pixs = 32
  }

  -- Some column data that will be returned.
  local columnData = {}

  -- Iterate through the buttons on the list, and fill in the column data.
  for i, button in ipairs(button_list) do
    -- All non-holds/rolls have a consistent state map based on animations.
    local tapActiveStates = {
      parts_per_beat = partsPerBeat,
      quanta = isClassic and {
        { per_beat = 1, states = classicTapStates[button] }
      } or {
        { per_beat = 1, states = tapStates[4] },
        { per_beat = 2, states = tapStates[8] },
        { per_beat = 3, states = tapStates[12] },
        { per_beat = 4, states = tapStates[16] },
        { per_beat = 6, states = tapStates[24] },
        { per_beat = 8, states = tapStates[32] },
        { per_beat = 12, states = tapStates[48] },
        { per_beat = 16, states = tapStates[64] }
      }
    }
     -- Holds and rolls use a dedicated state map. Start with the active one.
    local holdActiveStates = {
      parts_per_beat = partsPerBeat,
      quanta = isClassic and {
        { per_beat = 1, states = classicHoldStates[button] }
      } or {
        { per_beat = 1, states = holdStates[4] },
        { per_beat = 2, states = holdStates[8] },
        { per_beat = 3, states = holdStates[12] },
        { per_beat = 4, states = holdStates[16] },
        { per_beat = 6, states = holdStates[24] },
        { per_beat = 8, states = holdStates[32] },
        { per_beat = 12, states = holdStates[48] },
        { per_beat = 16, states = holdStates[64] }
      }
    }
    -- Set up the inactive quantum for easier math.
    local holdInactiveQuanta = {}
    for i, quantum in ipairs(holdActiveStates.quanta) do
      local states = {}
      for s, state in ipairs(quantum.states) do
        states[s] = state + 2
      end
      holdInactiveQuanta[i] = { per_beat = quantum.per_beat, states = states }
    end
    local holdInactiveStates = {
      parts_per_beat = partsPerBeat,
      quanta = holdInactiveQuanta
    }

    local holdTexture = holdButtons[button] .. "-hold 8x4.png"
    local rollTexture = holdButtons[button] .. "-roll 8x4.png"
    local tapTexture = tapButtons[button] .. " tap-note 8x8.png"
    local holdNote = {
      -- Start with the inactive states first.
      {
        state_map = holdInactiveStates,
        textures = { holdTexture },
        flip = "TexCoordFlipMode_None",
        disable_filtering = false,
        length_data = holdLengthData
      },
      {
        state_map = holdActiveStates,
        textures = { holdTexture },
        flip = "TexCoordFlipMode_None",
        disable_filtering = false,
        length_data = holdLengthData
      }
    }
    local rollNote = {
      -- Start with the inactive states first.
      {
        state_map = holdInactiveStates,
        textures = { rollTexture },
        flip = "TexCoordFlipMode_None",
        disable_filtering = false,
        length_data = holdLengthData
      },
      {
        state_map = holdActiveStates,
        textures = { rollTexture },
        flip = "TexCoordFlipMode_None",
        disable_filtering = false,
        length_data = holdLengthData
      }
    }
    columnData[i] = {
      width = 48,
      padding = 0,
      anim_uses_beats = true,
      anim_time = skin_parameters.colors.frame_cycle_beats,
      -- the tap states
      taps = {
        NoteSkinTapPart_Tap = {
          state_map = tapActiveStates,
          inactive_state_map = tapActiveStates,
          actor = Def.Sprite {
            Texture = tapTexture,
            -- Rotate the arrow with InitCommand. And shrink it.
            InitCommand = function(self) self:rotationz(tapRotations[button]):zoom(0.75) end
          }
        },
        NoteSkinTapPart_Mine = {
          state_map = tapActiveStates,
          inactive_state_map = tapActiveStates,
          actor = Def.Sprite {
            Texture = "mine 8x8.png"
          }
        },
        NoteSkinTapPart_Lift = {
          state_map = tapActiveStates,
          inactive_state_map = tapActiveStates,
          actor = Def.Sprite {
            Texture = "mine 8x8.png"
          }
        }
      },
      -- The hold states
      holds = {
        TapNoteSubType_Hold = holdNote,
        TapNoteSubType_Roll = rollNote
      },
      reverse_holds = {
        TapNoteSubType_Hold = holdNote,
        TapNoteSubType_Roll = rollNote
      }
    }
  end

  -- Return the column data along with some vivid stuff.
  return {
    columns = columnData,
    vivid_operation = skin_parameters.colors.frame_one_color == "Target"
  }
end
