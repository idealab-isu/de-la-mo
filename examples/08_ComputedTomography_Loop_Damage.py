import numpy as np

import delamo.CADwrap
from delamo.api import DelamoModeler 
from delamo.api import Layer
from delamo.api import bond_layers
from delamo import process

import os

# This example inserts delaminations derived from CT data into a
# model of a flat laminate. Unlike example 7, this example
# uses a loop to generate the layers, and demonstrates
# the loop unwrapping in the GEN PREDAMAGE step

# NOTE: As of 9/24/18 the result of the GEN PREDAMAGE step
# is not valid Python due to an error in deindentation of the
# "else" clause. This appears to be a redbaron bug


# Front matter
# ------------

# Read ACIS license key
acis_license = delamo.CADwrap.read_license_key(filename="license.dat")

# Initialize the DeLaMo model
DM=DelamoModeler.Initialize(globals(),
                            pointtolerancefactor=3.0,
                            normaltolerance=100e-4,
                            license_key=acis_license)

# CAD file to write and script file to generate. 
# The first parameter to output_filenames
# should always match the name of the original script
# with the ".py" stripped
(script_to_generate,
 cad_file_path_from_script,
 layer_boundary_template) = process.output_filenames("08_ComputedTomography_Loop_Damage",phase="ORIGINAL",process="DEFECT_INSERTION",apply_damage_script="08_ComputedTomography_add_damage.py")


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



# This script then generates both a CAD file and a Python script.
# The Python script can be run from Abaqus. It includes the
# initialization script referenced above, and also opens
# the CAD file and builds the model.


# Basic parameters

# Set layer thickness
thickness = 0.250 # mm

# Create the NURBS mold surface 
# over which the lamina will be laid
mold = delamo.CADwrap.NURBSd()
mold.degree_u = 3
mold.degree_v = 3
mold.knotvector_u = [ 0, 0, 0, 0, 1, 2, 3, 3, 3, 3 ]
mold.knotvector_v = [ 0, 0, 0, 0, 1, 2, 3, 3, 3, 3 ]
# read_ctrlpts would automatically set weight to 1.0
mold.read_ctrlpts("data/CP_Planar2.txt")

# Create and add point marker for fixed faced boundary condition
FixedPoint=[50.0,0.0,thickness/2.0] 

layup = [0,90,0,90,90,0,90,0]

# Create layers in a loop
for layernum in range(8):

    if layernum == 0:
        layer = Layer.CreateFromMold(DM,mold,delamo.CADwrap.OFFSET_DIRECTION,thickness,"Layer %d" % (layernum+1),LaminaSection,layup[layernum])

        layer1 = layer # save it so we can reference it later
        pass
    else:
        layer = Layer.CreateFromLayer(DM,prev_layer.gk_layer,delamo.CADwrap.OFFSET_DIRECTION,thickness,"Layer %d" % (layernum+1), LaminaSection,layup[layernum])
        pass
    
    layer.Finalize(DM)
    layer.MeshSimple(MeshElemTypes,meshsize,abqC.HEX_DOMINATED,abqC.SYSTEM_ASSIGN)
    FEModel.EncastreBC(name="FixedFace_%d" % (DM.get_unique()),
                       createStepName=ApplyForceStep.name,
                       region=layer.singlepart.GetInstanceFaceRegion(FixedPoint,DM.pointtolerance))

    if layernum != 0:
        bond_layers(DM,prev_layer, layer,defaultBC=delamo.CADwrap.BC_COHESIVE,CohesiveInteraction=CohesiveInteraction,ContactInteraction=ContactInteraction)
        pass

    # Update and add point marker for fixed faced boundary condition
    FixedPoint[2]+=thickness

    prev_layer=layer
    pass


ForceVector=[ 0.0, 0.0, -5e-4 ] # Units of MPa 
FEModel.SurfaceTraction(name="SurfTrac_%d" % (DM.get_unique()),
                        createStepName=ApplyForceStep.name,
                        region=layer1.singlepart.GetInstanceFaceRegionSurface((24.0,24.0,thickness*0.0),DM.pointtolerance),
                        distributionType=abqC.UNIFORM,
                        field='',
                        localCsys=None,
                        traction=abqC.GENERAL,
                        follower=abqC.OFF,
                        resultant=abqC.ON,
                        magnitude=np.linalg.norm(ForceVector),
                        directionVector=( (0,0,0), tuple(ForceVector/np.linalg.norm(ForceVector)) ),
                        amplitude=abqC.UNSET)


# You can have the job auto-start when the Python script is run
#DM.RunJob(BendingJob)


DM.Finalize(script_to_generate,cad_file_path_from_script)
