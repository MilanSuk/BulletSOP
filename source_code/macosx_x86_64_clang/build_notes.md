# Bullet SOP - Mac OS X
Notes for compiling Milan Suk's Bullet SOP for Houdini.

### Software Used
The initial build testing was done with these versions of software:
- Mac OS X 10.9.4
- Xcode 5.1.1
- Houdini 13.0.476
- Bullet 2.82 r2704

# Compile Instructions

### Getting Started
Make sure you've got the latest Xcode installed. Under Mavericks, to get the command line tools installed, you have to run `xcode-select --install`, and choose `Install` from the pop-up window.

After installing Houdini, you need to run it in a shell with all the Houdini environment variables setup correctly. If you don't have one you've created yourself, you can run `/Applications/Houdini 13.0.385/Houdini Shell.terminal`.

Lastly, run this command in a Terminal, to create our working directory: `mkdir ~/bullet_sop_dev`. Then `cd` into that directory, and we're ready to get started.

### Bullet Library
1. Download the Bullet library and Bullet SOP:
     1.  `curl -o ~/bullet_sop_dev https://bullet.googlecode.com/files/bullet-2.82-r2704.tgz`
     2. `cd ~/bullet_sop_dev; tar xvfz bullet-2.82-r2704.tgz; cd bullet-2.82-r2704`
     3. Download the Bullet SOP code to `~bullet_sop_dev`, and unzip/untar.
2. Patch `btSolverBody.h` <sup>[1]</sup>, though there may be a compiler flag to do this as well <sup>[2]</sup>:
     1. `cd ~/bullet_sop_dev/bullet-2.82-r2704/src/BulletDynamics/ConstraintSolver; cp ~/bullet_sop_dev/BulletSOP-master/source/macosx_x86_64_clang/btSolverBody.patch .`
     2. Run the patch with `patch < btSolverBody.patch`, and answer `Y` in response to `Reversed (or previously applied) patch detected!  Assume -R?`
3. Configure Bullet with Cmake (omits Demos, build static libs, etc...)
    1. `cd ~/bullet_sop_dev/bullet-2.82-r2704; cmake . -G "Unix Makefiles" -DBUILD_DEMOS:BOOL=OFF -DBUILD_SHARED_LIBS:BOOL=OFF -DCMAKE_OSX_ARCHITECTURES='x86_64' -DBUILD_MULTITHREADING=1 -DCMAKE_CXX_FLAGS="-fPIC" -DCMAKE_C_FLAGS="-fPIC"`
4. Build and install with `make -j4; sudo make install` (set `-j` to as many threads as you have/want to use)
5. Verify that the `/usr/local/bullet/` has properly populated `lib` and `include` dirs. This is where the `build.sh` script will look for the Bullet libraries and headers.

### Bullet SOP
1. Download the Bullet SOP code to `~bullet_sop_dev`, and unzip/untar.
2. `cd ~/bullet_sop_dev/BulletSOP-master/source/macosx_x86_64_clang`
3. Build it with `sh build.sh`. Ignore any errors that say `ld: library not found for -lc.o` 
4. Verify the plugin was installed via `ls -ltr /Users/$USER/Library/Preferences/houdini/13.0/dso/bulletSOP.dylib`
     1. You can also verify the library is not dynamically linked to the bullet library with `otool -L /Users/$USER/Library/Preferences/houdini/13.0/dso/bulletSOP.dylib` 
5. Install the icons via `cp ~/bullet_sop_dev/BulletSOP-master/icons /Users/$USER/Library/Preferences/houdini/13.0/config/Icons` (or any location included in `$HOUDINI_UI_ICON_PATH`, which isn't setup by default; just use your home dir preferences, unless you've got a reason to use another and add it to the icon path)

[1]: https://developer.blender.org/rB9c93c0bf08c0bc73d2c09e7f8c57721b557eb222   "Blender Changelog"
[2]: https://github.com/bulletphysics/bullet3/issues/164                        "Github Issue 164"
