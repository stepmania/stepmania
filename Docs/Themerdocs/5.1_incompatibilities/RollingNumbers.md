## Why RollingNumbers was changed.
The old RollingNumbers class did not give lua access to anything more than
the target number, requiring everything else to be set through metrics.  
Metrics are harder to make flexible than lua code, and cannot be changed
after the actor is loaded.  
The new version gives lua access to all its members, and more control over
how the parts are rendered.  Instead of a color to multiply the diffuse by
for the leading section, the Attribute feature of BitmapText is used.  
Because the attribute system bypasses diffuse, themers are advised to use
set_mult_attrs_with_diffuse if they use RollingNumbers with a diffuse color.

## Formatting
The format string used for RollingNumbers should not have padding built into
it.  The entire string returned after formatting will be considered the
"number" part and colored with the number attribute.  To pad the displayed
string with a glyph, set the leading glyph and the width to what you want.  

Example:
```
Def.RollingNumbers{
  Font= "Common Normal", InitCommand= function(self)
    self:set_chars_wide(9):set_text_format("%.0f")
      :set_leading_attribute{Diffuse= color("#ff0000")}
      :set_number_attribute{Diffuse= color("#0000ff")}
			:target_number(9999):set_approach_seconds(10)
  end
},
```

## No more metrics. (mostly)

RollingNumbers no longer has a lua function for loading from a metrics group.
Instead, there are lua functions for setting all its members.  
Some things like ScreenEvaluation and ScoreDisplay have an embedded
RollingNumbers member that is not easy to access in lua.  To ease setting of
embedded RollingNumbers, RollingNumbers will load from the metrics group
passed by its container.

These metrics should go away once the things that contain embedded
RollingNumbers are changed to give direct access.

### Metrics:
* Width
* LeadGlyph
* TextFormat
* ApproachSeconds
* Commify
* LeadingColor
* NumberColor

LeadingColor and NumberColor are the diffuse color to use for the attribute.
