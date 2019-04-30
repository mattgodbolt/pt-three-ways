* Raytracing three ways
  - C style / procedural?
  - Functional?
  - Trad OO style

## GOALS

* Demonstrate C++'s as a multi-paradigm language
* Compare and contrast approaches
  - performance
  - testability
  - ease of use

Raytracing: (or maybe path tracing?)
* fire ray from camera. intersect with a world. World is a hierarchy of objects, each bounded by a Bounding sphere/box
* Need to determine intersections. result of intersection:
  - position
  - surface normal
  - material
* Primitives:
  * Spheres
  * Planes
  * Meshes (maybe?)
  * Lights (maye)
* Materials
  - given u,v,w what;
    - emission
    - diffraction
    - specular
* scene defined in code(?)
  - saves on parsing a file etc unles
* or simplest thing ever? P 1 2 3 10 10 10 
  (maybe not for functional reasons?)
