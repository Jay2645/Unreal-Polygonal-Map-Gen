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

3. **Very important!** This project uses Unreal's FGameplayTags system that got brought out of experimental with Unreal Engine 4.15. In order for the system to work properly, you ***need*** to also enable the "GameplayTags" plugin in the Plugins menu (if it's not enabled already).

![Enable the GameplayTags Plugin](https://cloud.githubusercontent.com/assets/2058763/23337272/035e0798-fb9d-11e6-88ad-86426ac5cd6d.jpg)

Once you restart the Unreal Editor, you need to go to your Project Settings/GameplayTags and find `Gameplay Tag Table List`. Add two elements to the array, pointing to `/PolygonalMapGenerator/GameplayTags/BiomeTags` and `/PolygonalMapGenerator/GameplayTags/MapMetadataTags`.

![Add to the GameplayTags array](https://cloud.githubusercontent.com/assets/2058763/23337280/35faf58a-fb9d-11e6-9cbd-029c59499250.jpg)


If you don't see anything in the array, you need to go to enable your content browser to show plugin content. In the bottom-left of the content browser, you'll see "View Options". Make sure "Show Plugin Content" is checked, then go back into your Project Settings and see if you can add the GameplayTag tables.

![Enabling "Show Plugin Content"](https://cloud.githubusercontent.com/assets/2058763/23337287/4ac9d03a-fb9d-11e6-9138-263c55c5f059.jpg)


Once you've added the two tables to your GameplayTags array, restart the editor one more time (I know, it's a pain) so the GameplayTag array gets populated properly.

4. Once the editor has reloaded, you can either spawn in the `IslandMapGenerator` Actor raw, or create a Blueprint asset inheriting from it. Tweak the settings in `IslandData` to your liking -- you likely want to set `IslandType` to `RadialIsland` and `Island Point Selector` to `RandomPointGenerator`.

![An example settings panel](https://cloud.githubusercontent.com/assets/2058763/23337290/7bb8f068-fb9d-11e6-93a9-25b623c9b017.jpg)

Place the Actor in your level somewhere and call `CreateMap` on the `IslandMapGenerator`. This will create the actual map and notify you on completion. More details about how to use the data inside `IslandMapGenerator` are provided below.

![Blueprint example](https://cloud.githubusercontent.com/assets/2058763/23337302/bfde088c-fb9d-11e6-9ec4-7d5ef6792a6c.jpg)

![Output](https://cloud.githubusercontent.com/assets/2058763/23337305/cff8818e-fb9d-11e6-9290-b3c82c3bb004.jpg)

#Example Project

An example project using this system [can be found here](https://github.com/Jay2645/UnrealPolygonalMapGenExample).

#Changes from Source Article

There have been a few changes from the ActionScript source. There's nothing too drastic, just a couple changes for better ease-of-use:

* There has been an implementation of a "tag" system to provide more variety. This tag system uses Unreal's `FGameplayTag` class, which is fast, user-extensible, and can quickly and easily add classifications to things on the map. As an example, users can specify part of the map as a volcano by making a new GameplayTag like "MapData.MetaData.Volcano", which is then something that can be taken into account during Biome generation by extending the current classes. This tag system also replaces hardcoded booleans to determine whether part of the map is water, ocean, coast, etc.

* Most of the original ActionScript code was placed in a single class, `Map`, with a couple helper classes for the shape of the island (`IslandShape` in this project) and selecting which points to use (`PointGeneratior` in this project). This code has been further encapsulated, with the `Map` class (`IslandMapGenerator` here) being broken down into various stages. Each stage is its own class, which can be overridden and users can provide their own implementation if the default one isn't to their liking.

#How Maps Are Made

The actual map generation works by generating an array of points using a user-specified PointGeneratior (inheriting from UPointGenerator), which is triangulated using a [Delaunay Triangulation](https://en.wikipedia.org/wiki/Delaunay_triangulation). The Delaunay Triangulation is a dual graph of a [Voronoi Diagram](https://en.wikipedia.org/wiki/Voronoi_diagram), which provides us with 2 sets of points. This approach is called a "polygonal" map generation because we use the points of our Delaunay Triangulation as centers of polygons, with the vertices of that polygon given by the dual graph of the Voronoi Diagram.

Once a map is created, it will generate 3 different arrays:

* `FMapCenter`: This is the "center" of our polygon, and for all intents and purposes is treated as it being a polygon itself. This polygon can have any number (>=3) of sides, the vertices of which are contained in the `Corners` array.

* `FMapCorner`: This is a "vertex" of our polygon, which runs along the edge of the polygon. Rivers and such flow from FMapCorner to FMapCorner. A valid `FMapCorner` will ALWAYS be bordered by 3 `FMapCenter` structures, accessible from the `Touches` array. This means that a `FMapCorner` structure can be roughly associated with a triangle, with the three `FMapCenter` structures acting as vertices.

* `FMapEdge`: This is a helper class, and contains details on two different graphs. The Delaunay Triangulation can be found by looking at the DelaunayEdge, linking together two neighboring FMapCenters. The Voronoi Diagram can be found by looking at the VoronoiEdge, linking together two neighboring FMapCorners. Note that the order in which Center/Corner comes first cannot be guaranteed; sometimes it may be left-to-right, while other times it may be right-to-left.

Both the FMapCenter and FMapCorner contain an FMapData struct on the inside, which contains data specific to that point in space -- elevation, moisture, whether a polygon is water, etc.

#Using in Unreal

To use in Unreal, simply place an `AIslandMapGenerator` actor in your level and call `CreateMap` on it. `CreateMap` takes a delegate which will be called when the map is complete. After the map is complete, you can access map corners, centers, or edges by calling `GetCorner`, `GetCenter`, or `GetEdge`, all of which ask for an index of the element in question, which is bound between 0 and `GetCornerNum`, `GetCenterNum`, or `GetEdgeNum`.

Keep in mind that the above functions all return ***copies*** of objects; once modified, you need to call `UpdateCorner`, `UpdateCenter`, or `UpdateEdge`. You can also access an array of ***all** MapData objects across the entire graph; that is to say, it is an array of every single MapData object within every MapCorner and MapCenter. Doing this can give you a representation of the entire map as a whole, which can then be converted into a heightmap for whatever you need.

This system doesn't physically create anything inside the Unreal Engine itself, except for perhaps a debug diagram if the user chooses. It is up to the user to find or implement a system which can take the data from this graph and transform it into something tangible within the engine itself.

There is a class provided, `UPolygonalMapHeightmap`, which can create a heightmap from a `UPolygonMap`. To get this heightmap, call the function `CreateHeightmap()` and supply a function callback. This function will asynchronously generate a heightmap array, which can take several seconds. Once the array is complete, the callback will be called and you can safely access the heightmap array through `AIslandMapGenerator::GetHeightmap()`.

The `UPolygonalMapHeightmap` class provides a couple helper classes:

* `GetMapData()` provides a copy of the raw array of FMapData objects with size `FIslandData::Size` by `FIslandData::Size`, which the user can iterate over.

* `GetMapPoint()` takes in an integer X and Y value and safely outputs the FMapData object corresponding to that location. If that location is outside of the heightmap, it will output a "blank" FMapData object.

This array of FMapData objects can be turned into a 2D grayscale image by the user (using `FMapData::Elevation` to create the color value), or it can be used to create data points in a 3D voxel implementation.

# Credits

The original ActionScript code was released under the MIT Open Source license; this C++ port of the code is also released under the MIT Open Source license.

This project also uses [Bl4ckb0ne's C++ Delaunay Triangulation algorithm](https://github.com/Bl4ckb0ne/delaunay-triangulation), modified slightly to work with the Unreal Engine. This code is also used under the MIT License.

Conversion from Delaunay to Voronoi is (very loosely) based off of [Joseph Marshall's voronoi-c++ code](https://bitbucket.org/jlm/voronoi-c/src/b06aa9cccba6392d28ad7d7cae9a7361efb22c94?at=default), also modified under an MIT License.

The Markov Chain generator for generating strings from an array of FText was [based off of a version built in C# for the Unity game engine, created by Dave Carlile](http://crappycoding.com/2015/02/procedural-names/) used under the MIT license and converted to C++/modified to work with Unreal. This might be split off into a separate project at some point in the future, as it is only tangentally related to map generation.

Also used is a noise generator, which is heavily based on the [Jordan Peck's FastNoise code](https://github.com/Auburns/FastNoise) and is used under the MIT License. Minor modifications have been made to make it work with the Unreal Editor.

# Future Changes

* Right now, all operations take place on the main gameplay thread. In the future, it would be nice if this was done asynchronously, to allow for a "loading" bar or something similar.

* There is a lot of missing documentation, and the whole project could do with more comments clarifying how things work.

* More in-editor visualizations would be nice -- for example, seeing a view of the moisture distribution, or what biomes are where, or general debugging stuff like that.

* There should be a way to have the generator select a number of "player start" positions, so that the player always gets placed in a playable spot.

* Some of the map needs to be "cleaned up" -- there are random "spikes" which randomly occur along water edges for some reason, as an example.

* I still don't like how UPolygonMap handles its graph data objects. Essentially, right now everything is stored as an index to an array contained within the UPolygonMap object. I'd like to have them all be pointers to the actual elements in that array, but as it stands Unreal's `TArray` class (to my knowledge) doesn't support that behavior. Everything is thus accessed by indices to avoid any problems with mistakenly "cloning" graph objects.
