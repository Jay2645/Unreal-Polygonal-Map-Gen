#Polygonal Map Generator

This is a port of [the ActionScript code](https://github.com/amitp/mapgen2) of [Red Blob Games' *Polygonal Map Generation for Games*](http://www-cs-students.stanford.edu/~amitp/game-programming/polygon-map-generation/). This port is written in C++ and designed to work in Unreal Engine 4. This port works fully with games written in C++ as well as games that use Unreal Engine 4's "Blueprint" system.

The original ActionScript code was released under the MIT Open Source license; this C++ port of the code is also released under the MIT Open Source license.

This project also uses [Bl4ckb0ne's C++ Delaunay Triangulation algorithm](https://github.com/Bl4ckb0ne/delaunay-triangulation), modified slightly to work with the Unreal Engine. This code is also used under the MIT License. Conversion from Delaunay to Voronoi is (very loosely) based off of [Joseph Marshall's voronoi-c++ code](https://bitbucket.org/jlm/voronoi-c/src/b06aa9cccba6392d28ad7d7cae9a7361efb22c94?at=default), also modified under an MIT License.

The actual map generation works by generating an array of points using a user-specified PointGeneratior (inheriting from UPointGenerator), which is triangulated using a [Delaunay Triangulation](https://en.wikipedia.org/wiki/Delaunay_triangulation). The Delaunay Triangulation is a dual graph of a [Voronoi Diagram](https://en.wikipedia.org/wiki/Voronoi_diagram), which provides us with 2 sets of points. This approach is called a "polygonal" map generation because we use the points of our Delaunay Triangulation as centers of polygons, with the vertices of that polygon given by the dual graph of the Voronoi Diagram.

Once a map is created, it will generate 3 different arrays:

* `FMapCenter`: This is the "center" of our polygon, and for all intents and purposes is treated as it being a polygon itself.

* `FMapCorner`: This is a "vertex" of our polygon, which runs along the edge of the polygon. Rivers and such flow from FMapCorner to FMapCorner.

* `FMapEdge`: This is a helper class, and contains details on two different graphs. The Delaunay Triangulation can be found by looking at the DelaunayEdge, linking together two neighboring FMapCenters. The Voronoi Diagram can be found by looking at the VoronoiEdge, linking together two neighboring FMapCorners. Note that the order in which Center/Corner comes first cannot be guaranteed; sometimes it may be left-to-right, while other times it may be right-to-left.

Both the FMapCenter and FMapCorner contain an FMapData struct on the inside, which contains data specific to that point in space -- elevation, moisture, whether a polygon is water, etc.

#Changes from Source Article

There have been a few changes from the ActionScript source. There's nothing too drastic, just a couple changes for better ease-of-use:

* There has been an implementation of a "tag" system to provide more variety. This tag system uses Unreal's `FName` class, which is case-insensitive and built for fast lookup. This can, for example, allow users to specify part of the map as a "volcano," which is then something that can be taken into account during Biome generation.

* Most of the original ActionScript code was placed in a single class, `Map`, with a couple helper classes for the shape of the island (`IslandShape` in this project) and selecting which points to use (`PointGeneratior` in this project). This code has been further encapsulated, with the `Map` class (`IslandMapGenerator` here) being broken down into various stages. Each stage is its own class, which can be overridden and users can provide their own implementation if the default one isn't to their liking.

#Using in Unreal

This system doesn't physically create anything inside the Unreal Engine itself, except for perhaps a debug diagram if the user chooses. It is up to the user to find or implement a system which can take the data from this graph and transform it into something tangible within the engine itself.

One implementation could be to export the array of all FMapData nodes and then use those nodes to create a heightmap, either using the raw elevation data or have each point in the heightmap be weighted by the nearest FMapData points to it. These ideas are just examples; it is up to the user to ultimately decide how to use the data.
