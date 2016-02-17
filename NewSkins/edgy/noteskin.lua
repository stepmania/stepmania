return {
  -- the file to load the columns.
  notes = "notes.lua",
  -- the file to load the receptors
  layers = { "receptors.lua", "explosions.lua"},
  -- we do not support all of the buttons.
  supports_all_buttons = false,
  buttons = {"DownLeft", "UpLeft", "Center", "UpRight", "DownRight"},
  -- There is no appropriate fallback.
  fallback = "",
  -- Backup colors for when masks are used (routine).
  player_colors = {
    { 0, 0, 0.8, 1 },
    { 0.8, 0, 0, 1 },
    { 0.8, 0.8, 0, 1 },
    { 0.4, 0, 0.8, 1 },
    { 0.8, 0, 0.8, 1 },
    { 0.8, 0.4, 0, 1 }
  },
  -- Borrow the sane default skin parameters.
  skin_parameters = {
    explosions = {
      particles = true,
      particle_dist = 128,
      particle_life = 1,
      particle_size = 16,
      num_particles = 4,
      particle_blend = "BlendMode_WeightedMultiply"
    },
    colors = {
      color_style = "Classic",
      rhythm_quarter_color = "Red",
      frame_one_color = "Beat",
      frame_cycle_beats = 2
    }
  },
  -- Match every parameter with _info for option purposes.
  skin_parameter_info = {
    explosions = {
      translation = {
        en = {title = "Explosions", explanation = "Options for the explosion effects." }
      },
      particles = {
        translation = {
          en = {title = "Particles", explanation = "When set to true, particles will fly out when a note is hit." }
        }
      },
      particle_dist = {
        type = "int",
        min = 0,
        max = 2000,
        translation = {
          en = {title = "Particle Distance", explanation = "A higher number makes the particles fly further." }
        }
      },
      particle_life = {
        type = "float",
        min = .1,
        max = 10,
        translation = {
          en = {title = "Particle Life", explanation = "Number of seconds particles live for." }
        }
      },
      particle_size = {
        type = "int",
        min = 0,
        max = 2000,
        translation = {
          en = {title = "Particle Size", explanation = "A higher number makes the particles larger." }
        }
      },
      num_particles = {
        type = "int",
        min = 0,
        max = 64,
        translation = {
          en = {title = "Particle Count", explation = "A higher number makes the particles lag gameplay more." }
        }
      },
      particle_blend = {
        choices = {
          "BlendMode_Normal",
          "BlendMode_Add",
          "BlendMode_Subtract",
          "BlendMode_Modulate",
          "BlendMode_CopySrc",
          "BlendMode_AlphaMask",
          "BlendMode_AlphaKnockOut",
          "BlendMode_AlphaMultiply",
          "BlendMode_WeightedMulitply",
          "BlendMode_InvertDest",
          "BlendMode_NoEffect"
        },
        translation = {
          en = {
            title = "Particle Blend Mode",
            explanation = "Changes how the particles are blended.",
            choices = {
              "Normal",
              "Add",
              "Subtract",
              "Modulate",
              "Copy Source",
              "Alpha Mask",
              "Alpha Knock Out",
              "Alpha Multiply",
              "Weighted Multiply",
              "Invert Dest",
              "No Effect"
            }
          }
        }
      }
    },
    colors = {
      translation = {
        en = { title = "Colors", explanation = "Options for the colors used on this skin." }
      },
      color_style = {
        choices = {
          "Classic",
          "Rhythm"
        },
        translation = {
          en = {
            title = "Color Style",
            explanation = "Classic 5 panel colors or colors that go to a Rhythm?",
            choices = {
              "Classic",
              "Rhythm"
            }
          }
        }
      },
      rhythm_quarter_color = {
        choices = {
          "Blue",
          "Red"
        },
        translation = {
          en = {
            title = "Rhythm Quarter Color",
            explanation = "What color is the quarter note? Are you a groover or a professional?",
            choices = {
              "Blue",
              "Red"
            }
          }
        }
      },
      frame_one_color = {
        choices = {
          "Target",
          "Beat"
        },
        translation = {
          en = {
            title = "Frame One Graphic",
            explanation = "When does each note show the first frame?",
            choices = {
              "Across the Target Arrows",
              "Each Beat"
            }
          }
        }
      },
      frame_cycle_beats = {
        type = "int",
        choices = {
          1,
          2
        },
        translation = {
          en = {
            title = "Animation Length (Beats)",
            explanation = "How many beats to complete the animation?",
            choices = {
              "One",
              "Two"
            }
          }
        }
      }
    }
  }
}

