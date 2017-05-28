return {
	notes= "notes.lua",
	layers= {"receptors.lua", "explosions.lua"},
	supports_all_buttons= false,
	buttons= {"Left", "Down", "Up", "Right","UpLeft","UpRight","DownLeft","DownRight"},
	fallback= "default",
	skin_parameters= {
		treedee = false,
		vivid = false,
	},
	skin_parameter_info= {
		treedee ={
			translation={ en = {title="3D",explanation="Use the 3D version of the noteskin."} }
			
		},
		vivid={
			translation={ en = {title="Vivid",explanation="Use the vivid version of the noteskin. Rainbow!"}}
		}
			
	},
}
