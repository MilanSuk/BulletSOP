1. Installation

1.1 Plugin
For 64bit OS only! Copy library from "install/<yourOS>" to "$HOME/houdini12.5/dso/" for eg.:
WIN -> C:/Users/<Name>/Documents/houdini12.5/dso
UNX -> /home/<name>/houdini12.5/dso
OSX -> /Users/<Name>/Library/Preferences/houdini/12.5/dso


1.2 Icons
Icons created by Igor Bondar( jeybond7@gmail.com ). Thanks a lot!
Copy files from "install/icons" to "$HOME/houdini12.5/config/Icons/" for eg.:
WIN -> C:/Users/<Name>/Documents/houdini12.5/config/Icons
UNX -> /home/<name>/houdini12.5/config/Icons
OSX -> /Users/<Name>/Library/Preferences/houdini/12.5/config/Icons




2. Compilation
All source code files are in "source_code/src" folder.
Source code has been tested only on 64bit OS. It may works on 32bit OS too.

Important!
Zip file with source code package doesn't include Bullet Library. You can compile it by your self(check Compilation of Bullet Library section) or download pre-compile: https://www.dropbox.com/sh/y7t205a5akttky8/80Iv6SgWQr

Put compiled or downloaded(above) Bullet Library to:
source code files - BulletSOP_2_0_10_source_code/source_code/<your OS>/bullet-<version>/src
libraries files -   BulletSOP_2_0_10_source_code/source_code/<your OS>/bullet-<version>/lib

Firstly try to compile some of a Houdini example from "Houdini<version>\toolkit\samples\SOP" to make sure that you have all dev package and compiler compatible!
More info here: http://www.sidefx.com/docs/hdk12.5/hdk_intro.html




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
	Variable Name: houdini12
	Variable Value: 12.5.469

Also go to "Project settings -> Linker -> Command line" and change last line to path where plugin needs to be, for eg.:
-out:"C:/Users/<your user name>/Documents/houdini12.5/dso/SOP_bullet.dll"

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





3. Compilation of Bullet Library:
Download source code: http://code.google.com/p/bullet/downloads/list
Official guide: http://bulletphysics.org/mediawiki-1.5.8/index.php/Installation

After compilation copy:
bullet source code files - BulletSOP_2_0_10_source_code/source_code/<your OS>/bullet-<version>/src
bullet libraries files -   BulletSOP_2_0_10_source_code/source_code/<your OS>/bullet-<version>/lib


3.1. Windows:
I can recommend cmake-gui!


3.2 Linux
Use console commands:
cmake . -G "Unix Makefiles" -DBUILD_MULTITHREADING=1 -DCMAKE_CXX_FLAGS="-fPIC" -DCMAKE_C_FLAGS="-fPIC"
make


