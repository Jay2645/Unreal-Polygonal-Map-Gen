# Polygonal Map Generator

This is a port of [the JavaScript code](https://github.com/amitp/mapgen2) of [Red Blob Games' *Polygonal Map Generation for Games*](http://www-cs-students.stanford.edu/~amitp/game-programming/polygon-map-generation/). This port is written in C++ and designed to work in Unreal Engine 4 using the Unreal Engine 4 "Plugin" system. This port works fully with games written in C++ as well as games that use Unreal Engine 4's "Blueprint" system.

This version is designed to be a fairly barebones port; if you need extra features, [I've created a downstream fork](https://github.com/Jay2645/IslandGenerator) which supports things like procedural river names.

# Installation

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

If you don't have a `"Modules"` section, then that usually means that your project isn't set up for C++ yet. First, set your project up to handle C++ code, then you should see the `"Modules"` section. It's okay if you don't see a `"Plugins"` section, however -- just add one in there.

2. Open up your project in Unreal. If you get a prompt asking for an Unreal Engine version, you probably missed a comma somewhere in your .uproject file -- make sure all the commas and syntax are correct and try again. If Unreal does start to load, you might be told that your project is out of date, and the editor will ask if you want to rebuild your project. Go ahead and say yes so the plugin can be installed properly, keeping in mind that it might take a while to rebuild). After that, open up the Plugins menu, scroll down to the bottom, and ensure that the "PolygonalMapGenerator" plugin is enabled.

# Use

Everything you need to get started is in the "Content" folder of the Plugin. If you don't see it in the Unreal Engine editor, make sure to turn on "Show Plugin Content" in the view options in the bottom-right corner.

There's a couple "main" classes which make everything tick:

- The `IslandMap` class, which (as its name implies) generates islands. It uses a number of Unreal Engine data assets to drive the parameters for island creation; you can drag-and-drop in custom data assets with different parameters if you wanted to create something different.

- The `IslandMapMesh` class, which (again, like the name implies), generates islands and then creates a procedural mesh. This mesh is more of a proof-of-concept and quick example than anything robust and ready for gameplay; the class is basically there to show off how to use the data structures "in action," as it were.

The `IslandMapMesh` class is essentially a wrapper for the `IslandMap` class that just generates a mesh. The `IslandMap` class points to a collection of various data assets, which actually generate the island. If you wanted to implement custom water generation, a good way to do so would be to create a new C++ class that inherits from the `IslandWaters` class and then override whichever method you want to change. Inside Unreal, right-click inside a content folder, then navigate to Miscellaneous -> Data Asset and create a new instance of your custom `IslandWaters` data asset. Go back to the `IslandMap` and update the water data asset to point to the one you just made, and it should "just work."

# Credits

* The original code was released under the Apache 2.0 license; this C++ port of the code is also released under the Apache 2.0 license.

* Poisson Disc Sampling is created using code from the [Random Distribution Plugin](https://github.com/Xaymar/RandomDistributionPlugin) and used under the Apache 2.0 license.

* Delaunay Triangulation is created [using the MIT-licensed Delaunator](https://github.com/delfrrr/delaunator-cpp) and made accessible through a number of Unreal helper functions. Something that's fairly annoying: Delaunay Triangulation is [built into the engine](https://github.com/EpicGames/UnrealEngine/blob/08ee319f80ef47dbf0988e14b546b65214838ec4/Engine/Source/Editor/Persona/Private/AnimationBlendSpaceHelpers.h), but is only accessible from the Unreal Editor. The data structures aren't exposed to other modules or Blueprint, so you can't use it without linker errors when shipping your game. The Unreal Engine code has a different license, so a third-party library has to be used.