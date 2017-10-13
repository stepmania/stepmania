-- This is similar to one entry in "Graphics/NoteField layers.lua".
return function(button_list, stepstype, skin_parameters)
	return Def.ActorMultiVertex{
		Name= "Calipers", InitCommand= function(self)
			self:draworder(notefield_draw_order.explosion)
		end,
		WidthSetCommand= function(self, param)
			-- If the transform type is not None, the actor will use the average of
			-- the left and right columns.
			-- The column the noteskin places furthest left (most negative x value)
			-- is the left column, and furthest right is the right column.
			param.field:set_layer_transform_type(self, "FieldLayerTransformType_PosOnly")
			self:SetDrawState{Mode= "DrawMode_Quads"}
			local half_points= {
				{{0, 0}, {32, 0}},
				{{0, 0}, {-16, 32}},
				{{0, 0}, {-16, -32}},
			}
			local verts= {}
			local hw= param.width / 2
			for i, dir in ipairs{-1, 1} do
				local off= hw * dir
				for li= 1, #half_points do
					local line= half_points[li]
					local a= {line[1][1] + off, line[1][2]}
					local b= {line[2][1] * dir + off, line[2][2]}
					add_line_to_verts(a, b, verts, {1, 1, 1, 1}, 2, .5)
				end
			end
			self:SetVertices(verts)
		end,
	}
end
