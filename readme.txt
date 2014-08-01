1. Installation

1.1 Plugin
For 64bit OS only! Copy library from "install/<yourOS>" to "$HOME/houdini13/dso/" for eg.:
WIN -> C:/Users/<Name>/Documents/houdini13/dso
UNX -> /home/<name>/houdini13/dso
OSX -> /Users/<Name>/Library/Preferences/houdini/13/dso

Note: OSX version compiled by Chris Rydalch( crydalch@gmail.com )


1.2 Icons
Icons created by Igor Bondar( jeybond7@gmail.com ). Thanks a lot!
Copy files from "install/icons" to "$HOME/houdini13/config/Icons/" for eg.:
WIN -> C:/Users/<Name>/Documents/houdini13/config/Icons
UNX -> /home/<name>/houdini13/config/Icons
OSX -> /Users/<Name>/Library/Preferences/houdini/13/config/Icons




2. Compilation
All source code files are in "source_code/src" folder.
Source code has been tested only on 64bit OS. It may works on 32bit OS too.

Important!
Zip file with source code package doesn't include Bullet Library. You can compile it by your self(check Compilation of Bullet Library section) or download pre-compile: https://www.dropbox.com/sh/y7t205a5akttky8/80Iv6SgWQr

Put compiled or downloaded(above) Bullet Library to:
source code files - BulletSOP_2_0_10_source_code/source_code/<your OS>/bullet-<version>/src
libraries files -   BulletSOP_2_0_10_source_code/source_code/<your OS>/bullet-<version>/lib

Firstly try to compile some of a Houdini example from "Houdini<version>\toolkit\samples\SOP" to make sure that you have all dev package and compiler compatible!
More info here: http://www.sidefx.com/docs/hdk13/hdk_intro.html




2.1 Windows
For Windows there is Visual Studio 2008 project - BulletSOP_2_0_10_source_code/source_code/win64/bulletSOP2.sln
You can compile Debug and Release (both x64) version.

Project settings uses System Enviroment variable call "houdini12" or/and "houdini13" with value of actual Houdini version.
Setting Enviroment Variable:
- From the Desktop, right-click My Computer and click Properties.
- Click Advanced System Settings link in the left column.
- In the System Properties window click the Environment Variables button.
- Now to add a new System variable click on New
- Set name to houdini and value to your houdini version, for eg.:
	Variable Name: houdini13
	Variable Value: 13.0.376

Also go to "Project settings -> Linker -> Command line" and change last line to path where plugin needs to be, for eg.:
-out:"C:/Users/<your user name>/Documents/houdini13/dso/SOP_bullet.dll"

Major project setting were done in:
C/C++ -> General
C/C++ -> Command line
Linker -> Input
Linker -> Command line




2.2 Linux:
Run console and write:

cd /opt/hfs<your version>
source houdini_setup

cd <...>/BulletSOP_2_0_10_source_code/source_code/linux64
sh build.sh

houdini -foreground




2.3 OSX
Guide created by Chris Rydalch( crydalch@gmail.com )
Complete online version here: https://gist.github.com/crydalch/969bf739883b138f41e2

Make sure you've got the latest Xcode installed. Under Mavericks, to get the command line tools installed, you have to run xcode-select --install, and choose Install from the pop-up window.
After installing Houdini, you need to run it in a shell with all the Houdini environment variables setup correctly. If you don't have one you've created yourself, you can run /Applications/Houdini 13.0.385/Houdini Shell.terminal.
Lastly, run this command in a Terminal, to create our working directory: mkdir ~/bullet_sop_dev. Then cd into that directory, and we're ready to get started.

1) Download the Bullet SOP code to ~bullet_sop_dev, and unzip/untar.
2) cd ~/bullet_sop_dev/BulletSOP-master/source/macosx_x86_64_clang
3) Build it with sh build.sh. Ignore any errors that say ld: library not found for -lc.o
4) Verify the plugin was installed via ls -ltr /Users/$USER/Library/Preferences/houdini/13.0/dso/bulletSOP.dylib
5) Install the icons via cp ~/bullet_sop_dev/BulletSOP-master/icons /Users/$USER/Library/Preferences/houdini/13.0/config/Icons (or any location included in $HOUDINI_UI_ICON_PATH, which isn't setup by default; just use your home dir preferences, unless you've got a reason to use another and add it to the icon path)





3. Compilation of Bullet Library:
Download source code: http://code.google.com/p/bullet/downloads/list
Official guide: http://bulletphysics.org/mediawiki-1.5.8/index.php/Installation

After compilation copy:
bullet source code files - BulletSOP_2_0_12_source_code/source_code/<your OS>/bullet-<version>/src
bullet libraries files -   BulletSOP_2_0_12_source_code/source_code/<your OS>/bullet-<version>/lib



3.1. Windows:
I can recommend cmake-gui!



3.2 Linux
Use console commands:
cmake . -G "Unix Makefiles" -DBUILD_MULTITHREADING=1 -DCMAKE_CXX_FLAGS="-fPIC" -DCMAKE_C_FLAGS="-fPIC"
make



3.3 OSX
Guide created by Chris Rydalch( crydalch@gmail.com )
Complete online version here: https://gist.github.com/crydalch/969bf739883b138f41e2

1) Download the Bullet library and Bullet SOP:
	curl -o ~/bullet_sop_dev https://bullet.googlecode.com/files/bullet-2.82-r2704.tgz
	cd ~/bullet_sop_dev; tar xvfz bullet-2.82-r2704.tgz; cd bullet-2.82-r2704
	Download the Bullet SOP code to ~bullet_sop_dev, and unzip/untar.
2) Patch btSolverBody.h 1, though there may be a compiler flag to do this as well 2:
	cd ~/bullet_sop_dev/bullet-2.82-r2704/src/BulletDynamics/ConstraintSolver; cp ~/bullet_sop_dev/BulletSOP-master/source/macosx_x86_64_clang/btSolverBody.patch .
	Run the patch with patch < btSolverBody.patch, and answer Y to Reversed (or previously applied) patch detected! Assume -R?
3) Configure Bullet with Cmake (omits Demos, build static libs, etc...)
	cd ~/bullet_sop_dev/bullet-2.82-r2704; cmake . -G "Unix Makefiles" -DBUILD_DEMOS:BOOL=OFF -DBUILD_SHARED_LIBS:BOOL=OFF -DCMAKE_OSX_ARCHITECTURES='x86_64' -DBUILD_MULTITHREADING=1 -DCMAKE_CXX_FLAGS="-fPIC" -DCMAKE_C_FLAGS="-fPIC"
4) Build and install with make -j4; sudo make install (set -j to as many threads as you have/want to use)
5) Verify that the /usr/local/bullet/ has properly populated lib and include dirs. This is where the build.sh script will look for the Bullet libraries and headers.







