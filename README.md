#Polygonal Map Generator

This is a port of [the ActionScript code](https://github.com/amitp/mapgen2) of [Red Blob Games' *Polygonal Map Generation for Games*](http://www-cs-students.stanford.edu/~amitp/game-programming/polygon-map-generation/). This port is written in C++ and designed to work in Unreal Engine 4.

This port works fully with games written in C++ as well as games that use Unreal Engine 4's "Blueprint" system.

The map generation works by generating an array of points using a user-specified PointGeneratior (inheriting from UPointGenerator), which is triangulated using a [Delaunay Triangulation](https://en.wikipedia.org/wiki/Delaunay_triangulation). The Delaunay Triangulation is a dual graph of a [Voronoi Diagram](https://en.wikipedia.org/wiki/Voronoi_diagram), which provides us with 2 sets of points. This approach is called a "polygonal" map generation because we use the points of our Delaunay Triangulation as centers of polygons, with the vertices of that polygon given by the dual graph of the Voronoi Diagram.

Once a map is created, it will generate 3 different arrays:

* `FMapCenter`: This is the "center" of our polygon, and for all intents and purposes is treated as it being a polygon itself.

* `FMapCorner`: This is a "vertex" of our polygon, which runs along the edge of the polygon. Rivers and such flow from FMapCorner to FMapCorner.

* `FMapEdge`: This is a helper class, and contains details on two different graphs. The Delaunay Triangulation can be found by looking at the DelaunayEdge, linking together two neighboring FMapCenters. The Voronoi Diagram can be found by looking at the VoronoiEdge, linking together two neighboring FMapCorners. Note that the order in which Center/Corner comes first cannot be guaranteed; sometimes it may be left-to-right, while other times it may be right-to-left.

Both the FMapCenter and FMapCorner contain an FMapData struct on the inside, which contains data specific to that point in space -- elevation, moisture, whether a polygon is water, etc.

#Changes from Source Article

One way in which this implementation differs from the source article is that there has been an implementation of a "tag" system to provide more variety. This tag system uses Unreal's `FName` class, which is case-insensitive and built for fast lookup. This can, for example, allow users to specify a polygon (or just a vertex) as a "volcano," which is then something that can be taken into account during Biome generation.

#Using in Unreal

This system doesn't physically create anything inside the Unreal Engine itself, except for perhaps a debug diagram if the user chooses. It is up to the user to find or implement a system which can take the data from this graph and transform it into something tangible within the engine itself.

One implementation could be to export the array of all FMapData nodes and then use those nodes to create a heightmap, either using the raw elevation data or have each point in the heightmap be weighted by the nearest FMapData points to it. These ideas are just examples; it is up to the user to ultimately decide how to use the data.
