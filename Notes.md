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
