import numpy as np

import delamo.CADwrap
from delamo.api import DelamoModeler 
from delamo.api import Layer
from delamo.api import bond_layers
from delamo import process

import os
import sys

# Front matter
# ------------

# Read ACIS license key
acis_license = delamo.CADwrap.read_license_key(filename="license.dat")

# Initialize the DeLaMo model
DM=DelamoModeler.Initialize(globals(),
                                   pointtolerancefactor=100.0,
                                   normaltolerance=100e-4,
                                   license_key=acis_license)


# In manually generated scripts, always specify phase
# to be "ORIGINAL"
(script_to_generate,
 cad_file_path_from_script,
 layer_boundary_template) = process.output_filenames("11_Planar_Delam_Under_Stiffener",phase="ORIGINAL")

# When writing a DeLaMo script, you start by creating a 
# finite element initialization script. This is a 
# Python script for ABAQUS that defines your various parameters
# -- material properties, etc. as Python variables.  
# In this case they are stored in the "abqparams_CFRP.py" file
DM.abaqus_init_script("abqparams_CFRP.py",globals())

# The above call automatically inserts wrapped copies of variables
# defined in those scripts into the global variable space. Then you
# can reference those variables in this script


# The DM contains generates several sets of instructions
# for different phases of the finite element modeling process:
# DM.initinstrs  (initialization)
# DM.assemblyinstrs (model assembly)
# DM.bcinstrs (boundary conditions)
# DM.meshinstrs (meshing)

# All methods called from those variables will go generally be executed
# in the assemblyinstrs pool unless otherwise overridden. You can
# use e.g. DM.meshinstrs.rewrapobj() to get a reference to
# one of these variables that will execute in an alternate context.
#
# For example,
LaminateAssemblyMeshing=DM.meshinstrs.rewrapobj(LaminateAssembly)
# Creates a reference to the LaminateModel, for which method calls
# execute in the meshing context

ElemShape = abqC.HEX_DOMINATED
ElemTetShape = abqC.TET
ElemTechnique = abqC.SYSTEM_ASSIGN

# Basic parameters

# Set layer thickness
thickness = 0.199

# Create the NURBS mold surface 
# over which the lamina will be laid
mold = delamo.CADwrap.NURBSd()
mold.degree_u = 3
mold.degree_v = 3
mold.knotvector_u = [ 0, 0, 0, 0, 1, 2, 3, 3, 3, 3 ]
mold.knotvector_v = [ 0, 0, 0, 0, 1, 2, 3, 3, 3, 3 ]
# read_ctrlpts would automatically set weight to 1.0
mold.read_ctrlpts("data/CP_Planar1.txt")


# Create 1st layer                     
layer1 = Layer.CreateFromMold(DM,mold,delamo.CADwrap.OFFSET_DIRECTION,thickness,"Layer_1",LaminaSection,0)
layer1.Finalize(DM)
layer1.MeshSimple(MeshElemTypes,meshsize,ElemShape,ElemTechnique)

# Create and add point marker for fixed faced boundary condition
FixedPoint=[25.0,-20,0.1]

FEModel.EncastreBC(name="FixedFace_%d" % (DM.get_unique()),
                   createStepName=ApplyForceStep.name,
                   region=layer1.singlepart.GetInstanceFaceRegion(FixedPoint,DM.pointtolerance))


# Create 2nd layer
layer2 = Layer.CreateFromLayer(DM,layer1.gk_layer,delamo.CADwrap.OFFSET_DIRECTION,thickness,"Layer_2", LaminaSection,90)
layer2.Finalize(DM)
layer2.MeshSimple(MeshElemTypes,meshsize/1.8,ElemShape,ElemTechnique)

#bond_layers(DM,layer1, layer2)
bond_layers(DM,layer1, layer2,defaultBC=delamo.CADwrap.BC_COHESIVE,
            CohesiveInteraction=CohesiveInteraction,
            ContactInteraction=ContactInteraction,
            delaminationlist=["data/Delamination1_3D.csv"])

# Update and add point marker for fixed faced boundary condition
FixedPoint[2]+=thickness
FEModel.EncastreBC(name="FixedFace_%d" % (DM.get_unique()),
                   createStepName=ApplyForceStep.name,
                   region=layer2.singlepart.GetInstanceFaceRegion(FixedPoint,DM.pointtolerance))



# Create 3rd layer
layer3 = Layer.CreateFromLayer(DM,layer2.gk_layer,delamo.CADwrap.OFFSET_DIRECTION,thickness,"Layer_3",LaminaSection,-90)
layer3.Finalize(DM)
layer3.MeshSimple(MeshElemTypes,meshsize/1.8,ElemShape,ElemTechnique)

# The line below is intentionally COHESIVE. It imprints faces but sets that these faces are not delaminated.
#bond_layers(DM,layer2, layer3, delamo.CADwrap.BC_DEFAULT_COHESIVE, CohesiveInteraction, ContactInteraction, "data/Delamination1_3D.csv")
# The line below imprints and sets the delaminations
#bond_layers(DM,layer2, layer3, delamo.CADwrap.BC_DEFAULT_CONTACT, CohesiveInteraction, ContactInteraction, "data/Delamination1_3D.csv")
# The line below is regular bonding, i.e. no delaminations
bond_layers(DM,layer2, layer3)

# Update and add point marker for fixed faced boundary condition
FixedPoint[2]+=thickness
FEModel.EncastreBC(name="FixedFace_%d" % (DM.get_unique()),
                   createStepName=ApplyForceStep.name,
                   region=layer3.singlepart.GetInstanceFaceRegion(FixedPoint,DM.pointtolerance))


# Create 4th layer -- should use finemeshsize because this is where 
# delamination might go 
layer4 = Layer.CreateFromLayer(DM,layer3.gk_layer,delamo.CADwrap.OFFSET_DIRECTION,thickness,"Layer_4",LaminaSection,0)
layer4.Finalize(DM)
layer4.MeshSimple(MeshElemTypes,meshsize/2.0,ElemShape,ElemTechnique)

bond_layers(DM,layer3, layer4)

# Update and add point marker for fixed faced boundary condition
FixedPoint[2]+=thickness
FEModel.EncastreBC(name="FixedFace_%d" % (DM.get_unique()),
                   createStepName=ApplyForceStep.name,
                   region=layer4.singlepart.GetInstanceFaceRegion(FixedPoint,DM.pointtolerance))

# Create a stiffener
stiffener = Layer.CreateStiffenerFromLayer(DM,layer4.gk_layer,"stiffener", None,"data/HatStiffener.csv")

# Bond layer4 to stiffener
bond_layers(DM,layer4,stiffener)

# Create 5th layer over the layer 4 or the stiffener contour, if present
# ... for we just tell it to follow the layer 4 contour, which
# the stiffener automagically expanded
layer5 = Layer.CreateFromLayer(DM,layer4.gk_layer,delamo.CADwrap.OFFSET_DIRECTION,thickness,"Layer_5",LaminaSection,0)
layer5.Finalize(DM)

# Bond layer4 to layer5 and add delamination between them
layer5.MeshSimple(MeshElemTypes,meshsize/2.0,ElemTetShape,abqC.FREE) # Can use ElemTetShape here to allow tetragonal meshing
bond_layers(DM,layer4,layer5,defaultBC=delamo.CADwrap.BC_COHESIVE,
            CohesiveInteraction=CohesiveInteraction,
            ContactInteraction=ContactInteraction,
            delaminationlist=["data/Delamination1_3D.csv"])
    
# Update and add point marker for fixed faced boundary condition
FixedPoint[2]+=thickness
FEModel.EncastreBC(name="FixedFace_%d" % (DM.get_unique()),
                   createStepName=ApplyForceStep.name,
                   region=layer5.singlepart.GetInstanceFaceRegion(FixedPoint,DM.pointtolerance))


# Create 6th layer
layer6 = Layer.CreateFromLayer(DM,layer5.gk_layer,delamo.CADwrap.OFFSET_DIRECTION,thickness,"Layer_6",LaminaSection,90)
layer6.Finalize(DM)
layer6.MeshSimple(MeshElemTypes,meshsize,ElemShape,ElemTechnique)

bond_layers(DM,layer5, layer6)

# Update and add point marker for fixed faced boundary condition
FixedPoint[2]+=thickness
FEModel.EncastreBC(name="FixedFace_%d" % (DM.get_unique()),
                   createStepName=ApplyForceStep.name,
                   region=layer6.singlepart.GetInstanceFaceRegion(FixedPoint,DM.pointtolerance))


# Create 7th layer
layer7 = Layer.CreateFromLayer(DM,layer6.gk_layer,delamo.CADwrap.OFFSET_DIRECTION,thickness,"Layer_7",LaminaSection,-90)
layer7.Finalize(DM)
layer7.MeshSimple(MeshElemTypes,meshsize,ElemShape,ElemTechnique)

bond_layers(DM,layer6, layer7)

# Update and add point marker for fixed faced boundary condition
FixedPoint[2]+=thickness
FEModel.EncastreBC(name="FixedFace_%d" % (DM.get_unique()),
                   createStepName=ApplyForceStep.name,
                   region=layer7.singlepart.GetInstanceFaceRegion(FixedPoint,DM.pointtolerance))


# Create 8th layer
layer8 = Layer.CreateFromLayer(DM,layer7.gk_layer,delamo.CADwrap.OFFSET_DIRECTION,thickness,"Layer_8",LaminaSection,0)
layer8.Finalize(DM)
layer8.MeshSimple(MeshElemTypes,meshsize,ElemShape,ElemTechnique)

bond_layers(DM,layer7, layer8)

# Update and add point marker for fixed faced boundary condition
FixedPoint[2]+=thickness
FEModel.EncastreBC(name="FixedFace_%d" % (DM.get_unique()),
                   createStepName=ApplyForceStep.name,
                   region=layer8.singlepart.GetInstanceFaceRegion(FixedPoint,DM.pointtolerance))


ForceVector=[ 0.0, 0.0, -5e-2 ] # Units of MPa 
FEModel.SurfaceTraction(name="SurfaceTraction_%d" % (DM.get_unique()),
                        createStepName=ApplyForceStep.name,
                        region=layer1.singlepart.GetInstanceFaceRegionSurface((24.0,24.0,thickness*0.0),DM.pointtolerance),
                        distributionType=abqC.UNIFORM,
                        field='',
                        localCsys=None,
                        traction=abqC.GENERAL,
                        follower=abqC.OFF,
                        resultant=abqC.ON,
                        magnitude=np.linalg.norm(ForceVector),
                        directionVector=((0.0,0.0,0.0),tuple(ForceVector/np.linalg.norm(ForceVector))),
                        amplitude=abqC.UNSET)


# You can have the job auto-start when the Python script is run
#DM.RunJob(BendingJob)

DM.Finalize(script_to_generate,cad_file_path_from_script)
