BNGine
===================
Or how I learned to stop worrying and love...something.

An engine I'm writing in C++.  Makes use of other repos of mine: [3dbasics](https://github.com/Benjins/3dbasics) and [CppUtils](https://github.com/Benjins/CppUtils).  The CppUtils repo is added as a submodule.

Build Status
--------------

Linux Build: [![Linux Build Status](https://travis-ci.org/Benjins/BNGine.svg?branch=master)](https://travis-ci.org/Benjins/BNGine)

Installing
--------------
```
git clone --recursive https://github.com/Benjins/CppUtils.git
```
Or run the following after a clone:
```
git submodule update --init --recursive
```

Dependencies
-------------
Ubuntu: X11 and OpenGL development packages

Windows: Uhhh...OpenGL32.lib? Windows dependencies confuse me.

OS X: Ha ha, there's no Mac version. _yet_.

Building
-------------
*Windows*: Run ```build.bat runtime``` to build the runtime exe, or ```build.bat editor``` to build the editor.

*Linux*: Run ```./build.sh runtime``` to build the runtime exe, or ```./build.sh editor``` to build the editor.


TODO:

*(aka a bunch of ideas I really wish I could do.  And I just might)*

 * Editor
   - [X] Immediate-mode Gui
   - [X] Object selection
   - [X] Component editing 
   - [X] Saving/loading
   - [ ] better pan/orbit control
   - [ ] separate editors for different things:
    + [X] prefab editor
	+ [ ] material editor
	+ [ ] particle editor
 * Physics
   - [ ] basic Rigidbody simulation
   - [ ] Sphere-sphere collisions
   - [ ] Sphere-box collisons
   - [ ] Mesh colliders/GJK/Minkowski stuff?
 * Gui
   - [X] Have non-imm buttons
   - [ ] sliders (either imm or not, I just want some)
   - [ ] non-imm checkboxes
   - [ ] non-imm labels
   - [ ] non-imm string picker lists
   - [ ] non-imm frames/rects
   - [ ] non-imm scrollable rects
 * Graphics
   - [ ] better shading model, specular, etc.
   - [ ] cube maps, sky boxes, static reflections
   - [X] armature, animations
   - [ ] particle systems
 * Animation
   - [ ] Animation states, split track by time
   - [ ] Animation targets serialize
 * Networking
   - [ ] basic matchmaking server backend, just for testing?
   - [ ] Reliable and streaming packets, ACK's
   - [ ] Resolve id's between host/client
   - [ ] Player entity sync?
   - [ ] Packet design, spawning/destroying entities?
 * Metagen
   - [ ] Collect coroutines into discrimnated union
   - [X] Gui button actions, collect into enum and discriminated union
 * Script integration
   - [ ] yield/wait for seconds command
   - [X] Hotloading
   - [X] Use metaGen system to generate script files
 * MetaGen
   - [X] Parse attributse
   - [X] Parse enums
   - [X] Parse functions/methods
   - [ ] Custom serialize/editor methods?
   - [ ] Or, knowledge of disciminated unions and enums
   - [X] enums in editor
 * Misc
   - [X] Break out application entry point and platform entry point
   - [ ] Make XML unicode aware
   - [ ] Some kind of ref-counted unicode strings, like String/SubString
   - [ ] console menu with console commands
   - [ ] Substrings from Substrings