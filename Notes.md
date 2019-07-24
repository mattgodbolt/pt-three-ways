* Path tracing three ways
  - Trad OO style
  - Functional (optionals, variants for polymorph, free functions?)
  - Data oriented design

### GOALS

* Demonstrate C++'s as a multi-paradigm language
* Compare and contrast approaches
  - performance
  - testability
  - ease of use

## Path tracing

* fire ray from camera. intersect with a world. World is a hierarchy of objects, each bounded by a Bounding sphere/box
* Need to determine intersections. result of intersection:
  - position
  - surface normal
  - material
* Primitives:
  * Spheres
  * Planes (or maybe just triangles)
  * Meshes (maybe?)
  * Lights (maye)
* Materials
  - given u,v,w what;
    - emission
    - diffraction
    - specular
* scene defined in code(?)
  - saves on parsing a file etc unless
* or simplest thing ever? P 1 2 3 10 10 10 
  (maybe not for functional reasons?)


## Maybe...
* Show "pigeon" OO (like, no optional etc)
  * No mesh, just lots of triangles? that's kinda DoD?
* Show pure functional
* Show DoD (TBD)
* Show blend (C++ lets us have :allthethings:)
* Maybe strong types?