# smithBumpmapGenerator.dll

## plugin for generating bumpmaps (normal map) from RGB textures

Implements https://github.com/bahstrike/SmithPlugin as a unique plugin DLL.

## this experiment implements the following algorithm:
https://github.com/deepmind/lab/blob/cf2f5250e1a00ecce37b3480df28c3a5dcd08b57/engine/code/renderergl2/tr_image.c#L428

## attributions

*Honestly, it was not clear from Git Blame when the `RGBAtoNormal` function was added and by whom. Therefore I am uncertain as to the original author of the algorithm; although it is presumptuously Id Software, author of Quake III Arena.*

- DeepMind  (www.deepmind.com)
- Id Software, Inc.  (www.idsoftware.com)
- Google, Inc.  (www.google.com)
