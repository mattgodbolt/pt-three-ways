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
  * Triangles
* Materials
  - emission
  - diffraction - TODO maybe not?
  - specular - TODO
* Bounces TODO

## Maybe...
* Show "pigeon" OO (like, no optional etc)
  * No mesh, just lots of triangles? that's kinda DoD?
* Show "pure" functional
* Show DoD
* Show blend (C++ lets us have :allthethings:)
* Maybe strong types?
* Things to add after initial impl
  * (strong type?)
  * k-d tree
  * "intersection if nearer than XXX" optimisation?

Materials? Inline, or "OO" style?

## Timeline plan...

Written on 27th July
```
     July 2019
Su Mo Tu We Th Fr Sa
                  27
28 29 30 31
    August 2019
Su Mo Tu We Th Fr Sa
             1  2  3
 4  5  6  7  8  9 10
11 12 13 14 15 16 17
18 19 20 21 22 23 24
25 26 27 28 29 30 31
   September 2019
Su Mo Tu We Th Fr Sa
 1  2  3  4  5  6  7
 8  9 10 11 12 13 14
```

* Mondays: Compiler Explorer day
* Conference: Sep 15-20
* Ideally two weeks for practice, so all of September.
* A week clear to write talk, at least. So all versions need to be finished by Aug 24th
* Family visiting July 27th<->Aug 10th
* Away from computers 1st Aug <-> 6th Aug
* So! Only clear weeks: 11->17, 18->24
