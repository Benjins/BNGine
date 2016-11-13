BNGine
===================
Or how I learned to stop worrying and love...something.

An engine I'm writing in C++.  Makes use of other repos of mine: [3dbasics](https://github.com/Benjins/3dbasics) and [CppUtils](https://github.com/Benjins/CppUtils).  The CppUtils repo is added as a submodule, so run the following command:
```
git clone --recursive https://github.com/Benjins/CppUtils.git
```
Or run the following after a clone:
```
git submodule update --init --recursive
```

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
   - [ ] armature, animations
   - [ ] particle systems
 * Networking
   - [ ] basic matchmaking server backend, just for testing
   - [ ] Reliable and streaming packets, ACK's
   - [ ] Resolve id's between host/client
   - [ ] Player entity sync?
   - [ ] Packet design, spawning/destroying entities?
 * Metagen
   - [ ] Collect coroutines into discrimnated union
   - [X] Gui button actions, collect into enum and discriminated union
 * Script integration
   - [ ] yield/wait for seconds command
   - [ ] Hotloading
   - [X] Use metaGen system to generate script files
 * MetaGen
   - [X] Parse attributse
   - [ ] Parse enums
   - [X] Parse functions/methods
 * Misc
   - [X] Break out application entry point and platform entry point
   - [ ] Make XML unicode aware
   - [ ] Some kind of ref-counted unicode strings, like String/SubString