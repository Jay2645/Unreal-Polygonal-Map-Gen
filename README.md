# Polygonal Map Generator

[![PRs Welcome](https://img.shields.io/badge/PRs-welcome-brightgreen.svg?style=flat-square)](http://makeapullrequest.com) [![Maintenance](https://img.shields.io/badge/Maintained%3F-no-red.svg)](https://github.com/Jay2645/Unreal-Polygonal-Map-Gen/graphs/commit-activity)

For legal reasons I'm not allowed to work on this currently. However, I'm still able to review and OK pull requests if need be in the future. This should also work for Unreal 4.21 and (probably) 4.22.

## Created for Unreal Engine 4.21

![highresscreenshot00005](https://user-images.githubusercontent.com/2058763/50578306-17d8b880-0ded-11e9-8b8d-b7641fcd7479.png)

This is a port of [the JavaScript code](https://github.com/amitp/mapgen2) of [Red Blob Games' *Polygonal Map Generation for Games*](http://www-cs-students.stanford.edu/~amitp/game-programming/polygon-map-generation/).

This port is written in C++ and designed to work in Unreal Engine 4 using the Unreal Engine 4 "Plugin" system. This port works fully with games written in C++ as well as games that use Unreal Engine 4's "Blueprint" system.

It supports Unreal Engine version 4.21.x; you can *probably* use it for earlier/later versions of the Unreal Engine, but you may have to make slight modifications.

![highresscreenshot00000](https://user-images.githubusercontent.com/2058763/50556433-f0b7a500-0c8d-11e9-8636-21dc94e46699.png)

![highresscreenshot00001](https://user-images.githubusercontent.com/2058763/50556434-f0b7a500-0c8d-11e9-94c9-2e4dbf61520b.png)

![highresscreenshot00003](https://user-images.githubusercontent.com/2058763/50556436-f0b7a500-0c8d-11e9-92fe-0da8a22dbee8.png)

This version is designed to be a fairly barebones port; if you need extra features, [I've created a downstream fork](https://github.com/Jay2645/IslandGenerator) which supports things like procedural river names.

# Installation

## Access through Blueprint only:

If you only want to access everything through Blueprint, you can just clone this project into your \<Project Root\>/Plugins folder, where \<Project Root\> is where the .uplugin file is kept. You may have to create the Plugins folder if you haven't made it already. From there, you should be able to drag the pre-made Blueprints into your scene, create new custom Blueprints, or do whatever else you need.

## Access through C++ and Blueprints:

1. Make a `Plugins` folder at your project root (where the .uproject file is), if you haven't already. Then, clone this project into a subfolder in your Plugins directory. After that, open up your project's .uproject file in Notepad (or a similar text editor), and change the `"AdditionalDependencies"` and `"Plugins"` sections to look like this:

```
	"Modules": [
		{
			"Name": "YourProjectName",
			"Type": "Runtime",
			"LoadingPhase": "Default",
			"AdditionalDependencies": [
				<OTHER DEPENDENCIES GO HERE>
				"PolygonalMapGenerator"
			]
		}
	],
	"Plugins": [
		<OTHER PLUGINS GO HERE>
		{
			"Name": "PolygonalMapGenerator",
			"Enabled": true
		}
	]
```

> If you don't have a `"Modules"` section, then that usually means that your project isn't set up for C++ yet. First, set your project up to handle C++ code, then you should see the `"Modules"` section. It's okay if you don't see a `"Plugins"` section, however -- just add one in there.

> In the past, I have gotten linker errors by not listing this in the .uproject file when trying to package a project. People have said you don't need to do it nowadays, so you *might* be able to skip this step -- even so, I'm listing it here in case any problems arise.

2. Go into your source code folder, and you'll see a `<Project Name>.build.cs file. Open it up. Under either "PrivateDependencyModuleNames" or "PublicDependencyModuleNames" add a new entry called "PolygonalMapGenerator".

> This lets Unreal's Blueprint reflection system "see" your plugin and know to compile it before it compiles the rest of your code, so that you don't get weird linker errors when using things from the plugin.

3. Open up your project in Unreal. If you get a prompt asking for an Unreal Engine version, you probably missed a comma somewhere in your .uproject file -- make sure all the commas and syntax are correct and try again. If Unreal does start to load, you might be told that your project is out of date, and the editor will ask if you want to rebuild your project. Go ahead and say yes so the plugin can be installed properly (keeping in mind that it might take a while to rebuild).

4. Open up the Plugins menu, scroll down to the bottom, and ensure that the "PolygonalMapGenerator" plugin is enabled.

---

Once you have the project set up, you should be able to see it in the sidebar of the Content Browser; simply scroll down to "Polygonal Map Generator." You'll see 2 folders -- "Content" and "C++ Classes." If you don't see it in the Unreal Engine editor, make sure to turn on "Show Plugin Content" in the view options in the bottom-right corner.

* The "C++ Classes" folder contains, well, the C++ classes. You can right-click on them to create a new Blueprint or C++ class from the classes listed in that folder, or you can just do it the old-fashioned way, like you would inherit from Unreal's `Actor` class.

* The "Content" folder contains a bunch of content that can help you when making your own version of the plugin. If you wanted to, you could delete these files, but they give examples on how to set up a map generator and what the different classes in the plugin actually do.

# Use

There's a couple "main" classes which make everything tick:

* The `IslandMap` class, which (as its name implies) generates islands. It uses a number of Unreal Engine data assets to drive the parameters for island creation; you can drag-and-drop in custom data assets with different parameters if you wanted to create something different.

* The `IslandMapMesh` class, which (again, like the name implies), generates islands and then creates a procedural mesh. This mesh is more of a proof-of-concept and quick example than anything robust and ready for gameplay; the class is basically there to show off how to use the data structures "in action," as it were.

* Various data assets, which actually generate the island. Things like `IslandMoisture` handles moisture distribution, `IslandElevation` handles elevation distribution, etc. For the most part, [these match the files listed in the source repo](https://github.com/redblobgames/mapgen2).

> The main exception to `IslandWater`, which gives various examples on how to create islands of different shapes and sizes -- `IslandSquareWater` makes a square island (probably not very useful), `IslandRadialWater` makes a (mostly) circular island using overlapping Sine waves, and `IslandNoiseWater` uses simplex noise to create the shape of islands. Of these, I think `IslandNoiseWater` looks the best -- you can see pictures of the islands it generates at the top of this README file.

> There is also an additional file that's not in the source repo, under the "Mesh" folder -- `IslandMeshBuilder`. There's a couple subclasses `IslandSquareMeshBuilder` and `IslandPoissonMeshBuilder`. These lay out the actual vertices used in island creation; in the source repo, the mesh gets passed down with the points already added. These classes govern different ways to lay out the points -- `IslandSquareMeshBuilder` will do it in a uniform grid, whereas `IslandPoissonMeshBuilder` uses a poisson distribution to pseudo-randomly lay out all the points.

## Advanced

You can mess around with the data assets, creating new subclasses and overriding base methods. Unlike "normal" data assets, these assets *should* work with Blueprint, if needed. 

If you wanted to implement custom water generation, a good way to do so would be to create a new C++ class that inherits from the `IslandWaters` class and then override whichever method you want to change. Inside Unreal, right-click inside a content folder, then navigate to Miscellaneous -> Data Asset and create a new instance of your custom `IslandWaters` data asset. Go back to the `IslandMap` and update the water data asset to point to the one you just made, and it should "just work."

There's also a number of places to "hook" into the island generation code if you wanted to modify the existing logic of a certain step or add your own implementations.

As I mentioned, I tried to keep this port pretty close to the original. I've added a couple things for convenience, such as actual mesh generation as well as a data structure to keep track of rivers (`URiver`). However, while the rivers are placed, they do not get rendered and the underlying mesh is still the same -- you'll have to either roll your own option or look at [that downstream fork I mentioned earlier](https://github.com/Jay2645/IslandGenerator).

Speaking of mesh generation; it's not perfect -- it does its best to match each triangle to an individual biome for the purposes of assigning materials, but it comes out a bit jagged. 

# Credits

* The original code was released under the Apache 2.0 license; this C++ port of the code is also released under the Apache 2.0 license. Again, this was based on the [mapgen2](https://github.com/redblobgames/mapgen2) repository.

* Also included in this code is a port of the [DualMesh](https://github.com/redblobgames/dual-mesh) library; DualMesh is also licensed under Apache 2.0.

* Poisson Disc Sampling is created using code from the [Random Distribution Plugin](https://github.com/Xaymar/RandomDistributionPlugin) and used under the Apache 2.0 license.

* Delaunay Triangulation is created [using the MIT-licensed Delaunator](https://github.com/delfrrr/delaunator-cpp) and made accessible through a number of Unreal helper functions. Something that's fairly annoying: Delaunay Triangulation is [built into the engine](https://github.com/EpicGames/UnrealEngine/blob/08ee319f80ef47dbf0988e14b546b65214838ec4/Engine/Source/Editor/Persona/Private/AnimationBlendSpaceHelpers.h), but is only accessible from the Unreal Editor. The data structures aren't exposed to other modules or Blueprint, so you can't use it without linker errors when shipping your game. The Unreal Engine code has a different license, so a third-party library has to be used.

# Miscellaneous

## Why mapgen2? Why not [mapgen4](https://github.com/redblobgames/mapgen4)?

**Short answer:**

Because mapgen4 wasn't open-source yet when I started refactoring all the code from mapgen1 to mapgen2 (as far as I can tell, there is no mapgen3).

**Longer answer:**

I'm not the biggest fan of mapgen4. In the future, [I may borrow some concepts regarding river representation](http://simblob.blogspot.com/2018/10/mapgen4-river-representation.html), and I do [enjoy how the rivers get laid out.](https://www.redblobgames.com/x/1723-procedural-river-growing/) I'm also unhappy with the current way of determining elevation (it's just distance from the nearest coastline), and I [like the idea of having moisture determined by rainfall](http://simblob.blogspot.com/2018/09/mapgen4-rainfall.html).

However, I don't like how mapgen4 looks. It's a different approach to things -- it goes for a "handcrafted map" look, but that means there's a very distinct change division between hills and mountains, that's not very realistic. It's also designed in a way to let the user paint it; again, this is a cool idea, and one I might try to implement someday, but it's not the goal I was going for with this project. Ultimately, while there are some cool aspects to it (linked above), I prefer the way that mapgen2 feels (for the most part).
