# Purpose

item_scroller is a lua based alternative to ActorScroller.  It's designed
around actors controlled by lua objects to display whatever data the theme
wants.

The item_scroller creates a fixed number of items when the create_actors
function is called.  These are then set to the information elements based on
the current scroll position and their transform function is called to
position them.

There is no automated scrolling.  The items are only scrolled only when a
scroll function is called.

# Creation

Write an item metatable to control the items.  It must have create_actors,
set, and transform functions.  It can have any other functions that you need.
```lua
local item_mt= {
  __index= {
	-- create_actors must return an actor.  The name field is a convenience.
	create_actors= function(self, params)
	  self.name= params.name
		return Def.BitmapText{
		  Name= name, Font= "Common Normal", InitCommand= function(subself)
			  -- Setting self.container to point to the actor gives a convenient
				-- handle for manipulating the actor.
			  self.container= subself
			end}
	end,
	-- item_index is the index in the list, ranging from 1 to num_items.
	-- is_focus is only useful if the disable_wrapping flag in the scroller is
	-- set to false.
	transform= function(self, item_index, num_items, is_focus)
	  self.container:y(item_index)
	end,
	-- info is one entry in the info set that is passed to the scroller.
	set= function(self, info)
	  self.container:settext(info)
	end,
}}
```

Create a lua table with item_scroller_mt as its metatable.
```lua
local scroller= setmetatable({disable_wrapping= true}, item_scroller_mt)
```
If disable_wrapping is false or nil, the scroller will be in wheel mode,
which is designed a music wheel, where an item at the center is has focus and
info elements are repeated if there are more items than info elements, and
info elements wrap around when the scroller is scrolled to the top or bottom.

Create the actor for the scroller.
```lua
t[#t+1]= scroller:create_actors("foo", 5, item_mt, 0, 0)
```
This creates a scroller with 5 items, using item_mt as the metatable for each
item.  The base position of the scroller is 0, 0.

Create a table with the pieces of info you need the items to display.
```lua
local info_set= {"fin", "tail", "gorg", "lilk", "zos", "mink"}
```
item_scroller does not place any restrictions or requirements on what an
element of info_set can be.  The only guideline is that one element in
info_set will be passed to the set function in item_mt.  So one element
should contain everything an item needs to display that element.

After screen creation, use set_info_set to pass info_set to the scroller.
```lua
scroller:set_info_set(info_set, 1)
```
This passes the table of information to the scroller, and tells it to focus
on item 1.  The scroller then calls the set functions for the items, passing
each item one element from info_set.  Then the transform functions are called
to position the items.


# Scrolling

Call the scroll_to_pos and scroll_by_amount functions to scroll.
```lua
scroller:scroll_to_pos(3)
scroller:scroll_by_amount(2)
```
scroll_to_pos scrolls to put the given element at the focus position.
scroll_by_amount takes the current position and moves that many steps.

All scrolling is done in a single step.  Whether the scroller is moving the
elements one step or ten steps, the transform and set functions are only
called once for each item.


# item_scroller Functions

* item_scroller:create_actors(name, num_items, item_metatable, x, y, item_params)  
create_actors returns the actors for the item_scroller.  num_items is the
number of items to create for displaying the info.  num_items does not need
to be the number of elements in info_set.  num_items is the number of
elements you will be able to display simultanesously.  item_metatable is the
metatable to use when creating items.  x and y are the position to create the
item_scroller at.  item_params is a table that will be passed to the
create_actors function of item_metatable, so you can supply an item with
extra parameters it needs.

* item_scroller:set_info_set(info, pos)  
set_info_set sets the table of information that is displayed on the items.
pos sets which element in info is focused on.

* item_scroller:set_element_info(index, info)  
set_element_info is for changing an element in the info_set.  This will
update the item or items that are displaying that element by calling their
set function.

* item_scroller:scroll_to_pos(pos)  
Scrolls to pos as an absolute position.

* item_scroller:scroll_by_amount(amount)  
Scrolls by a relative amount.

* item_scroller:get_info_at_focus_pos()  
Returns the info element for the item at the focus position.
If disable_wrapping is true, this will probably not do what you want.  Keep
track of a cursor position and access the info table directly instead.

* item_scroller:get_actor_item_at_focus_pos()  
Returns the item at the focus position.
If disable_wrapping is true, this will probably not do what you want.  Keep
track of a cursor position and use get_items_by_info_index instead.

* item_scroller:get_items_by_info_index(index)  
Returns a table containing the items that are displaying the element of the
info_set that index points to.  This returns a table because multiple items
can be displaying the same info element.  Multiple items displaying the same
info element happens when disable_wrapping is false and there are more items
than elements in info_set (num_items is greater than #info_set).

* item_scroller:find_item_by_info(info)  
Similar to get_items_by_info_index, but takes an info element instead.  This
uses a simple equality test to find elements displaying the info element, so
if the info element is a table, it must be a table from info_set.


# Other Functions

* table.rotate_right(t, r)  
Rotates the elements in table t by r steps, moving each element to a lower
index.  Elements that would be moved to a 0 or negative index are rotated to
the (now empty) indices at the end of the table.

* table.rotate_left(t, l)  
Rotates the elements in table t by l steps, moving each element to a higher
index.  Elements that would be moved beyond the end of the table are moved to
the (now empty) lower indices.
