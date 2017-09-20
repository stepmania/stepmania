local trans_type= ...

local variants= {
	"shrinking",
	"growing",
	"growing_with_gaps",
	"growing_in_order",
	"growing_in_order_with_gaps",
	"growing_in_rows",
	"growing_in_rows_with_gaps",
	"growing_random",
	"growing_random_with_gaps",
	"growing_random_in_order",
	"growing_random_in_order_with_gaps",
	"growing_random_in_rows",
	"growing_random_in_rows_with_gaps",
	"falling_in_order",
	"falling_in_rows",
	"streaking",
	"streaking_random",
}
local picked_variant= "growing_in_rows_with_gaps"

local diamonds_across= math.random(8, 32)
local diamonds_high= math.random(4, 16)
local diamond_width= _screen.w / diamonds_across
local diamond_height= _screen.h / diamonds_high
local num_diagonals= diamonds_across + diamonds_high
local transition_time= 0.3
local diamond_side_len= math.sqrt((diamond_width^2) + (diamond_height^2))
local diagonal_start_x= diamond_width * diamonds_high * -.5
local diagonal_angle= math.atan2(diamond_height, diamond_width) / math.pi * 180
local unsquished_side_len= math.sqrt((diamond_height^2)*2) / 2

local function make_diagonal_line(x, y, angle)
	return Def.Quad{
		InitCommand= function(self)
			self:xy(x, y):rotationz(angle):setsize(_screen.h*2, 1)
				:diffuse{0, 0, 0, 1}
		end,
	}
end

local diamond_ratio= diamond_width / diamond_height
local function make_diamond(x, y, size, diam_com)
	return Def.ActorFrame{
		InitCommand= function(self)
			self:zoomx(diamond_ratio):xy(x, y)
		end,
		Def.Quad{
			InitCommand= function(self)
				self:setsize(size, size):diffuse{0, 0, 0, 1}:rotationz(45)
			end,
	}..diam_com}
end

local trans_frame= Def.ActorFrame{
	StartTransitioningCommand= function(self)
		if trans_type == "in" then
			self:playcommand("Full"):playcommand("Tween"):playcommand("Empty")
		else
			self:playcommand("Empty"):playcommand("Tween"):playcommand("Full")
		end
	end,
}
local variant_words= split("_", picked_variant)
if picked_variant == "shrinking" then
	local part_commands= {
		EmptyCommand= function(self)
			self:zoomy(0)
		end,
		FullCommand= function(self)
			self:zoomy(diamond_side_len)
		end,
		TweenCommand= function(self)
			self:linear(transition_time)
		end,
	}
	for diag= 0, num_diagonals-1 do
		local this_x= diagonal_start_x + (diag * diamond_width)
		trans_frame[#trans_frame+1]= make_diagonal_line(this_x, _screen.cy, diagonal_angle)
			.. part_commands
		trans_frame[#trans_frame+1]= make_diagonal_line(this_x, _screen.cy, -diagonal_angle)
			.. part_commands
	end
elseif variant_words[1] == "growing" then
	local part_commands= {
		EmptyCommand= function(self)
			self:zoom(0)
		end,
		FullCommand= function(self)
			self:zoom(1)
		end,
	}
	local random_flag= picked_variant:find("random")
	local in_order_flag= picked_variant:find("in_order")
	local in_rows_flag= picked_variant:find("in_rows")
	local with_gaps_flag= picked_variant:find("with_gaps")

	local part_size= unsquished_side_len
	local num_rows= diamonds_high * 2
	if with_gaps_flag then
		part_size= part_size * 2
		num_rows= diamonds_high
	end
	local num_diamonds= num_rows * diamonds_across
	local grow_time= transition_time / 2
	local offset_time= transition_time / num_diamonds
	if in_rows_flag then
		offset_time= transition_time / num_rows
	end

	local row_waits= {}
	local loop_internal= function(diam_y, diam_x, x, y)
		local new_part= make_diamond(x, y, part_size, part_commands)
		local diam_index= #trans_frame
		local row_index= num_rows - diam_y
		if random_flag then
			if in_order_flag then
				new_part[1].TweenCommand= function(self)
					local index= diam_index + (math.random() * num_diamonds / 2)
					local wait= index * offset_time
					self:sleep(wait):linear(grow_time)
				end
			elseif in_rows_flag then
				local wait= row_waits[diam_y]
				if not wait then
					wait= math.random() * num_rows * offset_time
					row_waits[diam_y]= wait
				end
				new_part[1].TweenCommand= function(self)
					self:sleep(wait):linear(grow_time)
				end
			else
				new_part[1].TweenCommand= function(self)
					local wait= math.random() * num_diamonds * offset_time
					self:sleep(wait):linear(grow_time)
				end
			end
		elseif in_order_flag then
			new_part[1].TweenCommand= function(self)
				local wait= diam_index * offset_time
				self:sleep(wait):linear(grow_time)
			end
		elseif in_rows_flag then
			new_part[1].TweenCommand= function(self)
				local wait= row_index * offset_time
				self:sleep(wait):linear(grow_time)
			end
		else
			new_part[1].TweenCommand= function(self)
				self:linear(transition_time)
			end
		end
		trans_frame[#trans_frame+1]= new_part
	end
	if with_gaps_flag then
		for diam_y= num_rows, 0, -1 do
			for diam_x= 0, diamonds_across do
				loop_internal(diam_y, diam_x,
					diam_x * diamond_width, diam_y * diamond_height)
			end
		end
	else
		for diam_y= num_rows, 0, -1 do
			local start_x= (diam_y % 2) * (diamond_width * .5)
			for diam_x= 0, diamonds_across do
				loop_internal(diam_y, diam_x,
					start_x + diam_x * diamond_width, .5 * diam_y * diamond_height)
			end
		end
	end
elseif variant_words[1] == "falling" then
	local in_order_flag= picked_variant:find("in_order")
	local in_rows_flag= picked_variant:find("in_rows")
	local part_size= unsquished_side_len
	local hide_y= _screen.h + diamond_height
	local num_rows= diamonds_high * 2
	local num_diamonds= num_rows * diamonds_across
	local fall_time= transition_time / 2
	local offset_time= transition_time / num_diamonds
	if in_rows_flag then
		offset_time= transition_time / num_rows
	end
	local diam_com= {
		EmptyCommand= function(self)
			self:zoom(1)
		end,
		FullCommand= function(self)
			self:zoom(1)
		end,
	}
	local loop_internal= function(diam_y, diam_x, start_x)
		local diam_index= #trans_frame
		local land_x= start_x + diam_x * diamond_width
		local land_y= .5 * diam_y * diamond_height
		local wait= 0
		if in_order_flag then
			wait= diam_index * offset_time
		elseif in_rows_flag then
			local row_index= num_rows - diam_y
			wait= row_index * offset_time
		else
			wait= math.random() * num_diamonds * offset_time
		end
		local part_com= {
			EmptyCommand= function(self)
				if trans_type == "in" then
					self:y(land_y + hide_y)
				else
					self:y(land_y - hide_y)
				end
			end,
			FullCommand= function(self)
				self:y(land_y)
			end,
			TweenCommand= function(self)
				self:sleep(wait):linear(fall_time)
			end,
		}
		local new_part= make_diamond(land_x, land_y, part_size, diam_com) ..
			part_com
		trans_frame[#trans_frame+1]= new_part
	end
	for diam_y= num_rows, 0, -1 do
		local start_x= (diam_y % 2) * (diamond_width * .5)
		for diam_x= 0, diamonds_across do
			loop_internal(diam_y, diam_x, start_x)
		end
	end
elseif variant_words[1] == "streaking" then
	local random_flag= picked_variant:find("random")
	local offset_time= transition_time / diamonds_across
	local fall_time= transition_time / 2
	local hide_y= _screen.h + diamond_height
	local part_size= unsquished_side_len
	local diam_com= {
		EmptyCommand= function(self)
			self:zoom(1)
		end,
		FullCommand= function(self)
			self:zoom(1)
		end,
	}
	local loop_internal= function(diam_x)
		local wait= diam_x * offset_time
		if random_flag then
			wait= math.random() * diamonds_across * offset_time
		end
		local new_part= Def.ActorFrame{
			InitCommand= function(self)
				self:x(diam_x * diamond_width)
			end,
			EmptyCommand= function(self)
				if trans_type == "in" then
					self:y(hide_y)
				else
					self:y(-hide_y)
				end
			end,
			FullCommand= function(self)
				self:y(0)
			end,
			TweenCommand= function(self)
				self:sleep(wait):linear(fall_time)
			end,
			Def.Quad{
				InitCommand= function(self)
					self:y(_screen.cy):setsize(diamond_width, _screen.h)
						:diffuse{0, 0, 0, 1}
				end,
			},
			make_diamond(0, 0, part_size, diam_com),
			make_diamond(0, _screen.h, part_size, diam_com),
		}
		trans_frame[#trans_frame+1]= new_part
	end
	for diam_x= 0, diamonds_across do
		loop_internal(diam_x)
	end
end

return trans_frame
