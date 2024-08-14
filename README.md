# Reshade VR Enhancer Mod (VREM)

## What is VREM ?

Reshade VREM is a reshade add-on that gives some improvement for DCS WORLD VR users (even if most of them are working in 2D).

It is the “son” of the 3Dmigoto mod, as the library is no longer working with openXR.

It provides the following features:

-   Label masking by cockpit frame
-   Reflection on A10C instrument glass can be suppressed
-   Atmosphere haze reduction
-   Enhancement of cockpit and outside colors
-   Sharpen filter for cockpit
-   Debanding for sea and sky
-   Own rotor hiding in cockpit view to avoir issue with reprojection (other rotor still visible)
-   AH64: TADS/PNVS video desactivation/activation in IHADSS
-   Settings can be saved

It is IC compatible, as no game files are modified. It should be compliant with any other reshade usage or addon.

It may eat some fps, but I did not have any reliable fps measurement tool working with the Varjo…

## Installation

**Download and install the reshade version** with full add_on support ( <https://reshade.me/>). Reshade must be installed for each version of DCS (ST or MT) you are using.

**Download the mod from here (to be updated) :** <https://www.digitalcombatsimulator.com/fr/files/3305420/>

**Install the mod in DCS folders**

It is best to use Jsgme or Ovgme (the provided zip is compliant with them) but otherwise:

-   unzip the mod package
-   copy the “bin” and “bin-mt” folders (that are in the mod folder) into the DCS installation directory. You should have a file “**reshade DCS VREM.addon64**” and a folder “**shaderreplace**” in the folders in which DCS.exe is.

## UnInstallation

It is best to use Jsgme or Ovgme (the provided zip is compliant with them) but otherwise:

-   delete “**reshade DCS VREM.addon64**” and the folder “**shaderreplace**” in the folders “bin” and “bin-mt” of DCS installation

## Using the mod

See [https://forum.dcs.world/topic/356128-reshade-vr-enhancer-mod-vrem/](https://forum.dcs.world/topic/356128-reshade-vr-enhancer-mod-vrem/ %20)

## Acknowledgements

The mod is using code from

Crosire <https://github.com/crosire/reshade>

FransBouma <https://github.com/FransBouma/ShaderToggler>

ShortFuse <https://github.com/clshortfuse/renodx>

Once again, thanks for helping me in the reshade Discord, without their kind answers to my stupid questions, nothing would have been possible !

Sharpen algorithm and lot of code for it taken from here : [https://astralcode.blogspot.com/2018...ing-of_13.html](https://astralcode.blogspot.com/2018/11/reverse-engineering-rendering-of_13.html)

FXAA algorithm and lot of code for it taken from here : <http://blog.simonrodriguez.fr/articles/30-07-2016_implementing_fxaa.html>
