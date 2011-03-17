PortKit: StepMania 4
--------------------------------------------------------------------------------
[What is a PortKit?]
PortKits are designed to help facilitate rapid porting of themes from one
version of StepMania to another.

[How to use a PortKit/ThemeKit]
In the metrics.ini file for the theme, you'll need to add (or change) the lines
between the underscores:
___________________________
[Global]
FallbackTheme=_portKit-xxx
___________________________
where "xxx" represents the PortKit you're using.
If the theme you're working with has this set already, you should perform the
edit in the theme that FallbackTheme is pointing to, unless it is _fallback or
default. (Please don't edit the sm-ssc fallback and default themes.)

[What about this PortKit in particular?]
PortKit: StepMania 4 replicates the default StepMania 4 theme on top of
sm-ssc's _fallback. In addition to the metrics (mainly taken wholesale from
the default SM4 theme with IsBaseTheme set to 0), there are a few other things
added in:

* BGAnimations folder
  _missing isn't in the fallback theme currently, plus some themes rely on the
  various frame scripts that live there.
* Graphics folder
  Some themes rely on _frames 1d.
* Scripts folder
  Since sm-ssc uses a different branch system, the old one is provided
  here for compatibility.
* Sounds folder
  No one should ever have to deal with missing sounds when importing a SM4
  theme. Ever.