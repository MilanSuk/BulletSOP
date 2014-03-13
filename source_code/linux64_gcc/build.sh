#run this script in console with "sh build.sh"
#plugin directory: /home/ubuntu/houdini12.5/dso

BULLET_INC=bullet-2.82-r2704/src
BULLET_LIB=bullet-2.82-r2704/lib

rm ../src/*.o
rm ../src/*.a
echo "Removing old files done"


#compiles files
#note: "-l c.o" creates error, but this is fine, because *.o file was made it
hcustom -l c.o ../src/EThread.cpp

hcustom -l c.o ../src/SHelper.cpp
hcustom -l c.o ../src/SShape.cpp
hcustom -l c.o ../src/SObject.cpp
hcustom -l c.o ../src/SConstraint.cpp
hcustom -l c.o ../src/SForce.cpp
hcustom -l c.o ../src/SImpact.cpp
hcustom -l c.o ../src/SEmit.cpp

hcustom -l c.o -I $BULLET_INC ../src/BRigidBody.cpp
hcustom -l c.o -I $BULLET_INC ../src/BImpact.cpp
hcustom -l c.o -I $BULLET_INC ../src/BConstraint.cpp
hcustom -l c.o -I $BULLET_INC ../src/BWorld.cpp
hcustom -l c.o -I $BULLET_INC ../src/BCollisionFilter.cpp
hcustom -l c.o -I $BULLET_INC ../src/SOP_Solver.cpp

hcustom -l c.o ../src/SOP_Prepare.cpp
hcustom -l c.o ../src/SOP_Transform.cpp
hcustom -l c.o ../src/SOP_Build.cpp
hcustom -l c.o ../src/SOP_Modify.cpp
hcustom -l c.o ../src/SOP_Loader.cpp
hcustom -l c.o ../src/SOP_ExtraCells.cpp


#renames
mv ../src/EThread.o ../src/libEThread.o.a

mv ../src/SHelper.o ../src/libSHelper.o.a
mv ../src/SShape.o ../src/libSShape.o.a
mv ../src/SObject.o ../src/libSObject.o.a
mv ../src/SConstraint.o ../src/libSConstraint.o.a
mv ../src/SForce.o ../src/libSForce.o.a
mv ../src/SImpact.o ../src/libSImpact.o.a
mv ../src/SEmit.o ../src/libSEmit.o.a

mv ../src/BRigidBody.o ../src/libBRigidBody.o.a
mv ../src/BImpact.o ../src/libBImpact.o.a
mv ../src/BConstraint.o ../src/libBConstraint.o.a
mv ../src/BWorld.o ../src/libBWorld.o.a
mv ../src/BCollisionFilter.o ../src/libBCollisionFilter.o.a
mv ../src/SOP_Solver.o ../src/libSOP_Solver.o.a

mv ../src/SOP_Prepare.o ../src/libSOP_Prepare.o.a
mv ../src/SOP_Transform.o ../src/libSOP_Transform.o.a
mv ../src/SOP_Build.o ../src/libSOP_Build.o.a
mv ../src/SOP_Modify.o ../src/libSOP_Modify.o.a
mv ../src/SOP_Loader.o ../src/libSOP_Loader.o.a
mv ../src/SOP_ExtraCells.o ../src/libSOP_ExtraCells.o.a


#links *.a file together
hcustom -L ../src -L $BULLET_LIB -I $BULLET_INC -l SOP_Loader.o -l SOP_Solver.o -l SOP_Prepare.o -l SOP_Transform.o -l SOP_Build.o -l SOP_Modify.o -l SOP_ExtraCells.o -l SObject.o -l SConstraint.o -l SForce.o -l SImpact.o -l SShape.o -l SEmit.o -l SHelper.o -l BWorld.o -l BulletMultiThreaded -l EThread.o -l BRigidBody.o -l BConstraint.o -l BImpact.o -l BCollisionFilter.o -l BulletDynamics -l BulletCollision -l LinearMath ../src/bulletSOP.cpp


echo "Compilation done"







