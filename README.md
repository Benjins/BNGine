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

 - Editor
   [X] Immediate-mode Gui
   [X] Object selection
   [X] Component editing 
   [X] Saving/loading
 - Physics
   [ ] Rigidbody simulations
   [ ] Collision detection
   [ ] Mesh colliders
 - Script integration
   [ ] yield/wait for seconds command
   [ ] Hotloading
   [ ] Use metaGen system to generate script files
 - metaGen
   [ ] Parse attributse
   [ ] Parse enums
   [ ] Parse functions/methods
 - misc
   [ ] Break out application entry point and platform entry point