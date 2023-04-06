# PopcornFX Assets - O3DE Multiplayer Sample

The Multiplayer Sample includes nine visual FX assets created with PopcornFX. This README documents the these FX assets so that you can customize them or deconstruct them to learn how to build PopcornFX assets of your own.

The PopcornFX project containing all the source assets can be found in the following directory:
`<multiplayer-sample-root>/PopcornFX/PopcornProject.pkproj`.

## Requirements

The **PopcornFX** Gem depends on the following additional Gems:

* **Atom**
* **Script Canvas**
* **EMotionFX**
* **PhysX**
* **LyShine**
* **Starting Point Input**

## General information

Some of the FX are split into multiple parts (for example, Blast, Projectile, and Explosion). Splitting the FX makes it easier to integrate the FX into varied gameplay scenarios.

Many of the FX have a **Color Intensity** property that you can modify to adjust the emissive effect. The emissive effect requires that the **PostFX Layer** and **Bloom** components are added to the scene.

![Post FX Layer and Bloom components](./readme_media/image1.png)

## Armor Power Up

The Armor Power Up is not a particle system, but rather mesh and shader asset.

![](./readme_media/image35.gif width=420)

The effect is triggered by the **Slider** property on the `ArmorPowerUp.material`.

![](./readme_media/image16.png)

The `Script_ArmorPowerUp.scriptcanvas` is a test Script Canvas that loops the Armor Power Up FX.

## Bubble Gun

The Bubble Gun VFX is split into the following three parts:

| **Blast** | **Projectile** | **Explosion** |
| - | - | - |
| ![](./readme_media/image24.gif | width=300) | ![](./readme_media/image25.gif | width=300) | ![](./readme_media/image29.gif | width=300) |

The following properties are available to adjust the look of Bubble Gun VFX:

<img src="readme_media/image26.png" width=420>

| Property | Description |
| :-- | :-- |
| **Global Scale** | Scales the size of the VFX. |
| **Distortion Power** | Amount of distortion effect. |
| **Trail Lifetime** | The lifetime of the smoke. Higher values increase the length of the trail. |

## Defense Turret

The Defense Turret VFX is split into the following three parts:

| **Blast** | **Projectile** | **Explosion** |
| - | - | - |
| <img src="readme_media/image31.gif" width=350> | <img src="readme_media/image33.gif" width=350> | <img src="readme_media/image32.gif" width=350> |

The following properties are available to adjust the look of the Defense Turret VFX:

<img src="readme_media/image9.png" width=420>

| Property | Description |
| :-- | :-- |
| **Color 1, Color 2** | Sets the color and alpha values for the VFX. |
| **Global Scale** | Scales the size of the VFX. |
| **Color Intensity** | Sets the intensity of the emissive effect. |

The Explosion FX needs to be oriented with the **Impact Normal** attribute. You can set this up in Script Canvas.

<img src="readme_media/image12.png" width=420>

## Energy Ball Trap

The Energy Ball Trap VFX is split into the following three parts:

**Build up**
**Projectile**
**Explosion**

<img src="readme_media/image37.gif" width=800></br>

The following properties are available to adjust the look of the Energy Ball Trap VFX:

<img src="readme_media/image13.png" width=420>

| Property | Description |
| :-- | :-- |
| **Global Scale** | Scales the size of the VFX. |
| **Color Intensity** | Sets the intensity of the emissive effect. |

For a smooth transition between the three parts, each part needs to start at the same frame the previous part ends. You can trigger the FX in Script Canvas.

<img src="readme_media/image10.png" width=420>

## Energy Collector

The Energy Collector is a simple single emitter. It just needs to be placed accurately on an entity and triggered by Script Canvas.

<img src="readme_media/image36.gif" width=420></br>

The following properties are available to adjust the look of Energy Collector VFX:

<img src="readme_media/image18.png" width=420>

| Property | Description |
| :-- | :-- |
| **Color 1, Color 2** | Sets the color and alpha values for the VFX. |
| **Distortion Power** | Sets the amount of the distortion effect. |
| **Global Scale** | Scales the size of the VFX. |

## Jump Pad

The Jump Pad is a simple single emitter. It just needs to be placed accurately in a level and triggered by Script Canvas.

<img src="readme_media/image34.gif" width=420></br>

The following properties are available to adjust the look of Energy Collector VFX:

<img src="readme_media/image27.png" width=420>

| Property | Description |
| :-- | :-- |
| **Burst** | Sets duration of the launch. |
| **Color** | Sets the color and alpha of the VFX. |
| **Brightness** | Scales the glow effect of the VFX. |
| **Light Range** | Sets the reach of the glow effect of the VFX. |
| **Global Size** | Scales the size of the VFX. |

To trigger the Jump Pad VFX, the **Launch** property must be toggled. It doesn't matter if the boolean is true or false. The VFX will trigger if the state of the **Launch** property changes.

<img src="readme_media/image30.png" width=420></br>

The test Script Canvas `script_VFXTestTriggerJumpPad` loops the Jump Pad VFX.

## Laser Gun

<img src="readme_media/image21.gif" width=500><br>

The Laser Gun VFX is split into the following three parts:

| **Blast** | **Ray** | **Explosion** |
| - | - | - |
| <img src="readme_media/image17.gif" width=350> | <img src="readme_media/image22.gif" width=350> | <img src="readme_media/image11.gif" width=350> |

The following properties are available to adjust the look of Laser Gun VFX:

<img src="readme_media/image6.png" width=420>

| Property | Description |
| :-- | :-- |
| **Color 1, Color 2** | Sets the color and alpha values for the VFX. |
| **Global Size** | Scales the size of the VFX. |
| **Max Length** | Sets the distance at which the explosion occurs if the **Hit Position** is further away than the **Max Length** value. This avoids spawning too many particles if the **Hit Position** is too far away. |
| **Laser Lifetime** | Sets the lifespan of the ray. |
| **Hit Position** | Sets the world position of the end of the ray. |
| **Hit Normal** | The normal of the impact surface that sets the orientation of the explosion. |
| **Distortion Power** | Sets the amount of the distortion effect. |

## Malfunctioning Shield Generator

The Malfunctioning Shield Generator VFX is split into the following three parts:

**Idle**
**Build up**
**Explosion**

For a smooth transition between the three parts, each part needs to start at the same frame the previous part ends. You can trigger the FX in Script Canvas.

<img src="readme_media/image38.gif" width=800></br>

The following properties are available to adjust the look of Malfunctioning Shield Generator VFX:

<img src="readme_media/image14.png" width=420>
<img src="readme_media/image15.png" width=420><br>

| Property | Description |
| :-- | :-- |
| **Distortion Power** | Sets the amount of distortion effect on the **Explosion** emitter. |
| **Frequency** | Sets an approximate time (in seconds) between two electric bursts on the **Idle** emitter. |

## Speed Power Up

The Speed Power Up VFX is one emitter but three stages that are triggered automatically.

**Build Up**
**Idle**
**Fade Out**

<img src="readme_media/image23.gif" width=420></br>

The following properties are available to adjust the look of Speed Power Up VFX:

<img src="readme_media/image19.png"><br>

| Property | Description |
| :-- | :-- |
| **Global Scale** | Scales the size of the VFX. |
| **Particles Count** | Sets the trail particle count. Higher values give the vFX a more opaque look. |
| **Color Wind** | Sets the color and alpha of the wind trail. |
| **Duration** | Sets an approximate time (in seconds) for the **Idle** stage of the VFX. |
