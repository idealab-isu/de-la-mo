import numpy as np

import delamo.CADwrap
from delamo.api import DelamoModeler 
from delamo.api import Layer
from delamo.api import bond_layers
from delamo import process

import os

# Front matter
# ------------

# Fix file path
os.chdir(os.path.dirname(os.path.realpath(__file__)))

# Read ACIS license key
acis_license = delamo.CADwrap.read_license_key(filename="license.dat")

# Initialize the DeLaMo model
DM=DelamoModeler.Initialize(globals(),
                            pointtolerancefactor=100.0,
                            normaltolerance=100e-4,
                            license_key=acis_license)

# CAD file to write and script file to generate. 
# The first parameter to output_filenames
# should always match the name of the original script
# with the ".py" stripped
(script_to_generate,
 cad_file_path_from_script,
 layer_boundary_template) = process.output_filenames("09_CohesiveLayer",phase="ORIGINAL",process=None)


# When writing a DeLaMo script, you start by creating a 
# finite element initialization script. This is a 
# Python script for ABAQUS that defines your various parameters
# -- material properties, etc. as Python variables.  
# In this case they are stored in the "abqparams_CFRP.py" and "abqparams_CFRP_cohesive.py" files
DM.abaqus_init_script("abqparams_CFRP.py",globals())
DM.abaqus_init_script("abqparams_CFRP_cohesive.py",globals())

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
FEModelMeshing=DM.meshinstrs.rewrapobj(FEModel)
# Creates a reference to the FEModel, for which method calls
# execute in the meshing context



# This script then generates both a CAD file and a Python script.
# The Python script can be run from Abaqus. It includes the 
# initialization script referenced above, and also opens 
# the CAD file and builds the model. 

# Basic parameters

# Set layer thickness
thickness = 0.199

# Set cohesive layer thickness
cohesivethickness = 0.001

# Create the NURBS mold surface 
# over which the lamina will be laid
mold = delamo.CADwrap.NURBSd()
mold.degree_u = 3
mold.degree_v = 3
mold.knotvector_u = [ 0, 0, 0, 0, 1, 2, 3, 3, 3, 3 ]
mold.knotvector_v = [ 0, 0, 0, 0, 1, 2, 3, 3, 3, 3 ]
# read_ctrlpts would automatically set weight to 1.0
#mold.read_ctrlpts("data/ControlPointsCurved1.txt")
mold.read_ctrlpts("data/CP_Planar1.txt")


# Create 1st layer                     
layer1 = Layer.CreateFromMold(DM,mold,delamo.CADwrap.OFFSET_DIRECTION,thickness,"Layer_1",LaminaSection,0)
layer1.Finalize(DM)

layer1.MeshSimple(MeshElemTypes,meshsize,abqC.HEX_DOMINATED,abqC.SYSTEM_ASSIGN)

# Create and add point marker for fixed faced boundary condition
FixedPoint=[-20.0,-24.0,thickness] 

FEModel.EncastreBC(name="FixedFace_%d" % (DM.get_unique()),
                   createStepName=ApplyForceStep.name,
                   region=layer1.singlepart.GetInstanceFaceRegion(FixedPoint,DM.pointtolerance))


# Create cohesive layer
layer12cohesive = Layer.CreateFromLayer(DM,layer1.gk_layer,delamo.CADwrap.OFFSET_DIRECTION,cohesivethickness,"Layer12cohesive", CohesiveSection,0) # Orientation doesn't really matter since we have made it isotropic

# Update and add point marker for fixed faced boundary condition
FixedPoint[2]+=cohesivethickness


# Create 2rd layer
layer2 = Layer.CreateFromLayer(DM,layer12cohesive.gk_layer,delamo.CADwrap.OFFSET_DIRECTION,thickness,"Layer_2",LaminaSection,90)
layer2.Finalize(DM)

layer2.MeshSimple(MeshElemTypes,meshsize/1.8,abqC.HEX_DOMINATED,abqC.SYSTEM_ASSIGN)

bond_layers(DM,layer1, layer2,cohesive_layer=layer12cohesive,defaultBC=delamo.CADwrap.BC_COHESIVE_LAYER,delaminationlist=["data/nasa-delam12-1.csv"])
#layer12cohesive.Finalize(DM) bond_layers() does the finalize step so we don't have to 
# ... but therefore the meshing of the cohesive layer must be done AFTER bond_layers
layer12cohesive.MeshCohesive(meshsize/1.8,abqC.HEX_DOMINATED)
# Update and add point marker for fixed faced boundary condition
FixedPoint[2]+=thickness
FEModel.EncastreBC(name="FixedFace_%d" % (DM.get_unique()),
                   createStepName=ApplyForceStep.name,
                   region=layer2.singlepart.GetInstanceFaceRegion(FixedPoint,DM.pointtolerance))


# Can define a "Surface" that is visible in the Abaqus output database
layer1.singlepart.fe_part.Surface(name="ForceSurface",
                                  side1Faces=layer1.singlepart.GetPartFace((24.0,24.0,thickness*0),DM.pointtolerance))

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
