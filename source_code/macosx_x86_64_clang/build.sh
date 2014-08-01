#run this script in console with "sh build.sh"
#plugin directory: /home/ubuntu/houdini12.5/dso

BULLET_INC=/usr/local/include/bullet
BULLET_LIB=/usr/local/lib

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
cp ../src/EThread.o ../src/libEThread.o.a

cp ../src/SHelper.o ../src/libSHelper.o.a
cp ../src/SShape.o ../src/libSShape.o.a
cp ../src/SObject.o ../src/libSObject.o.a
cp ../src/SConstraint.o ../src/libSConstraint.o.a
cp ../src/SForce.o ../src/libSForce.o.a
cp ../src/SImpact.o ../src/libSImpact.o.a
cp ../src/SEmit.o ../src/libSEmit.o.a

cp ../src/BRigidBody.o ../src/libBRigidBody.o.a
cp ../src/BImpact.o ../src/libBImpact.o.a
cp ../src/BConstraint.o ../src/libBConstraint.o.a
cp ../src/BWorld.o ../src/libBWorld.o.a
cp ../src/BCollisionFilter.o ../src/libBCollisionFilter.o.a
cp ../src/SOP_Solver.o ../src/libSOP_Solver.o.a

cp ../src/SOP_Prepare.o ../src/libSOP_Prepare.o.a
cp ../src/SOP_Transform.o ../src/libSOP_Transform.o.a
cp ../src/SOP_Build.o ../src/libSOP_Build.o.a
cp ../src/SOP_Modify.o ../src/libSOP_Modify.o.a
cp ../src/SOP_Loader.o ../src/libSOP_Loader.o.a
cp ../src/SOP_ExtraCells.o ../src/libSOP_ExtraCells.o.a


#links *.a file together
hcustom -L ../src -L $BULLET_LIB -I $BULLET_INC -l SOP_Solver.o -l SOP_Prepare.o -l SOP_Transform.o -l SOP_Build.o -l SOP_Modify.o -l SOP_Loader.o -l SOP_ExtraCells.o -l SObject.o -l SConstraint.o -l SForce.o -l SImpact.o -l SShape.o -l SEmit.o -l SHelper.o  -l BulletMultiThreaded -l BWorld.o -l EThread.o -l BRigidBody.o -l BConstraint.o -l BImpact.o -l BCollisionFilter.o -l BulletDynamics -l BulletCollision -l LinearMath ../src/bulletSOP.cpp


echo "Compilation done"







