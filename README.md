#Polygonal Map Generator

This is a port of [the ActionScript code](https://github.com/amitp/mapgen2) of [Red Blob Games' *Polygonal Map Generation for Games*](http://www-cs-students.stanford.edu/~amitp/game-programming/polygon-map-generation/). This port is written in C++ and designed to work in Unreal Engine 4 using the Unreal Engine 4 "Plugin" system. This port works fully with games written in C++ as well as games that use Unreal Engine 4's "Blueprint" system.

#Installation

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

# Credits

The original ActionScript code was released under the MIT Open Source license; this C++ port of the code is also released under the MIT Open Source license.

Delaunay Triangulation is created [using Delaunator](https://github.com/delfrrr/delaunator-cpp) and made accessible through a number of Unreal helper functions. Something that's fairly annoying: Delaunay Triangulation is [built into the engine](https://github.com/EpicGames/UnrealEngine/blob/08ee319f80ef47dbf0988e14b546b65214838ec4/Engine/Source/Editor/Persona/Private/AnimationBlendSpaceHelpers.h), but is only accessible from the Unreal Editor. The data structures aren't exposed to other modules or Blueprint, so you can't use it. The Unreal Engine code has a different license, so a third-party library has to be used.