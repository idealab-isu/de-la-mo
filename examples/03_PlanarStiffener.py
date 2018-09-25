import numpy as np

import delamo.CADwrap
from delamo.api import DelamoModeler 
from delamo.api import Layer
from delamo.api import bond_layers
from delamo import process


import os
import os.path  

# Front matter
# ------------

# Read ACIS license key
acis_license = delamo.CADwrap.read_license_key(filename="license.dat")

# Initialize the DeLaMo model
DM=DelamoModeler.Initialize(globals(),
                            pointtolerancefactor=3.0,
                            normaltolerance=100e-4,
                            license_key=acis_license)

# The name of the script file to generate and
# the name of the CAD file to write are returned
# by process.output_filenames()

# The first parameter to output_filenames
# should always match the name of the original script
# with the ".py" stripped

# In manually generated scripts, always specify phase
# to be "ORIGINAL"
(script_to_generate,
 cad_file_path_from_script,
 layer_boundary_template) = process.output_filenames("03_PlanarStiffener",phase="ORIGINAL")



# When writing a DeLaMo script, you start by creating a
# finite element initialization script. This is a
# Python script for ABAQUS that defines your various parameters
# -- material properties, etc. as Python variables.
# In this case they are stored in the "abqparams_CFRP.py" file
DM.abaqus_init_script("abqparams_CFRP.py",globals())

# The above call automatically inserts wrapped copies of variables
# defined in those scripts into the global variable space. Then you
# can reference those variables in this script


# The Delamo model generates several lists of instructions
# for different phases of the finite element modeling process:
# DM.initinstrs  (initialization)
# DM.assemblyinstrs (model assembly)
# DM.bcinstrs (boundary conditions)
# DM.meshinstrs (meshing)

# All methods called from the variables defined in an initscript
# will generally be executed in the assemblyinstrs pool unless
# otherwise overridden. You can
# use e.g. DM.meshinstrs.rewrapobj(variable) to get a reference to
# a variable that will execute in an alternate context.
#
# For example,
LaminateAssemblyMeshing=DM.meshinstrs.rewrapobj(LaminateAssembly)
# Creates a reference to the LaminateAssembly, for which method calls
# execute in the meshing context

# Basic parameters


# Set delamination on or off
is_delam = False

# set stiffener on or off
is_stiffener = True

# Set layer thickness
thickness = 0.199

# Create the NURBS mold surface 
# over which the lamina will be laid
mold = delamo.CADwrap.NURBSd()
mold.degree_u=3
mold.degree_v=3
knot_vector_u = [0, 0, 0, 0, 1, 2, 3, 3, 3, 3]
knot_vector_v = [0, 0, 0, 0, 1, 2, 3, 3, 3, 3]
mold.knotvector_u=knot_vector_u
mold.knotvector_v=knot_vector_v
mold.read_ctrlpts("data/CP_Planar1.txt")
default_weight = 1.0
weights = [default_weight for i in range(0, mold.ctrlpts_len())]
mold.weights=weights



# Create 1st layer
                        
layer1 = Layer.CreateFromMold(DM,mold,delamo.CADwrap.OFFSET_DIRECTION,thickness,"Layer 1",LaminaSection,0)
layer1.Finalize(DM)
layer1.MeshSimple(MeshElemTypes,meshsize,abqC.HEX_DOMINATED,abqC.SYSTEM_ASSIGN)

# Create and add point marker for fixed faced boundary condition
FixedPoint=[-25.0,-20.0,0.1] 

FEModel.EncastreBC(name="FixedFace_%d" % (DM.get_unique()),
                   createStepName=ApplyForceStep.name,
                   region=layer1.singlepart.GetInstanceFaceRegion(FixedPoint,DM.pointtolerance))

# Create 2nd layer

layer2 = Layer.CreateFromLayer(DM,layer1.gk_layer,delamo.CADwrap.OFFSET_DIRECTION,thickness,"Layer 2", LaminaSection,-45)
layer2.Finalize(DM)
layer2.MeshSimple(MeshElemTypes,meshsize/1.8,abqC.HEX_DOMINATED,abqC.SYSTEM_ASSIGN)

bond_layers(DM,layer1, layer2)

# Update and add point marker for fixed faced boundary condition
FixedPoint[2]+=thickness
FEModel.EncastreBC(name="FixedFace_%d" % (DM.get_unique()),
                   createStepName=ApplyForceStep.name,
                   region=layer2.singlepart.GetInstanceFaceRegion(FixedPoint,DM.pointtolerance))


# Create 3rd layer
layer3 = Layer.CreateFromLayer(DM,layer2.gk_layer,delamo.CADwrap.OFFSET_DIRECTION,thickness,"Layer 3",LaminaSection,45)
layer3.Finalize(DM)
layer3.MeshSimple(MeshElemTypes,meshsize/1.8,abqC.HEX_DOMINATED,abqC.SYSTEM_ASSIGN)

bond_layers(DM,layer2, layer3, defaultBC=delamo.CADwrap.BC_COHESIVE,
            CohesiveInteraction=CohesiveInteraction,
            ContactInteraction=ContactInteraction,
            delaminationlist = [ "data/Delamination1_3D.csv" ])

# Update and add point marker for fixed faced boundary condition
FixedPoint[2]+=thickness
FEModel.EncastreBC(name="FixedFace_%d" % (DM.get_unique()),
                   createStepName=ApplyForceStep.name,
                   region=layer3.singlepart.GetInstanceFaceRegion(FixedPoint,DM.pointtolerance))

# Create 4th layer -- should use finemeshsize because this is where 
# delamination might go 
layer4 = Layer.CreateFromLayer(DM,layer3.gk_layer,delamo.CADwrap.OFFSET_DIRECTION,thickness,"Layer 4",LaminaSection,90)
layer4.Finalize(DM)
layer4.MeshSimple(MeshElemTypes,meshsize/2.0,abqC.HEX_DOMINATED,abqC.SYSTEM_ASSIGN)

bond_layers(DM,layer3, layer4)
# Update and add point marker for fixed faced boundary condition
FixedPoint[2]+=thickness
FEModel.EncastreBC(name="FixedFace_%d" % (DM.get_unique()),
                   createStepName=ApplyForceStep.name,
                   region=layer4.singlepart.GetInstanceFaceRegion(FixedPoint,DM.pointtolerance))

if is_stiffener:
    # Create a stiffener
    stiffener = Layer.CreateStiffenerFromLayer(DM,layer4.gk_layer,"stiffener",None,"data/HatStiffener.csv")   # delamo.CADwrap.OFFSET_DIRECTION is implicit

    # Bond layer4 to stiffener
    bond_layers(DM,layer4,stiffener,defaultBC=delamo.CADwrap.BC_COHESIVE,
                CohesiveInteraction=CohesiveInteraction)
    layer5mold=stiffener
    pass
else:
    layer5mold=layer4
    pass

# NOTE: Right now stiffener doesn't show up due to a bug
# in the layer bonding, but the root cause is trying to
# be too automatic in the layering

# Suggested new layering sematics
# Can provide list of mold objects:
#   Each object records what portion of it has been used as a mold
#   for something. Any creation operation uses all available surfaces
#   compatible with the shape of the layer being created. 

layer5 = Layer.CreateFromLayer(DM,layer4.gk_layer,delamo.CADwrap.OFFSET_DIRECTION,thickness,"Layer 5",LaminaSection,90)
layer5.Finalize(DM)
###!!!*** For some reason, with delamination AND stiffener, Abaqus
### won't mesh layer5 with regular hex elements.
## But TET seems to work (?) 
if is_stiffener and is_delam:
    # Should use finemeshsize, but get prohibitive number of elements
    # (500000) in that case
    layer5.MeshSimple(MeshElemTypes,meshsize/2.0,abqC.TET,abqC.SYSTEM_ASSIGN)
    pass
else:
    layer5.MeshSimple(MeshElemTypes,meshsize/2.0,abqC.HEX_DOMINATED,abqC.SYSTEM_ASSIGN)
    pass

if is_stiffener:
    # Bond layer 5 to stiffener
    # Temporarily disabled because C++ code does not yet support this

    #bond_layers(DM,layer5,stiffener,CohesiveInteraction) #,"Delamination1.csv
    pass

# Bond layer 5 to layer 4 with optional delamination 
if False and is_delam:
    bond_layers(DM,layer4, layer5, defaultBC=delamo.CADwrap.BC_COHESIVE,
                CohesiveInteraction=CohesiveInteraction,
                ContactInteraction=ContactInteraction,
                delaminationlist=["data/Delamination1_1D.csv"])  # This bonds faces that are marked with COHESIVEBC and creates CONTACTBC for faces marked that way
    pass
else:
    bond_layers(DM,layer4, layer5,defaultBC = delamo.CADwrap.BC_COHESIVE,
                CohesiveInteraction=CohesiveInteraction,
                ContactInteraction=ContactInteraction) #CohesiveInteraction,ContactInteraction) # This bonds faces that are marked with COHESIVEBC and creates CONTACTBC for faces marked that way
    pass
    

# Update and add point marker for fixed faced boundary condition
FixedPoint[2]+=thickness
FEModel.EncastreBC(name="FixedFace_%d" % (DM.get_unique()),
                   createStepName=ApplyForceStep.name,
                   region=layer5.singlepart.GetInstanceFaceRegion(FixedPoint,DM.pointtolerance))

# Create 6th layer
layer6 = Layer.CreateFromLayer(DM,layer5.gk_layer,delamo.CADwrap.OFFSET_DIRECTION,thickness,"Layer 6",LaminaSection,45)
layer6.Finalize(DM)
layer6.MeshSimple(MeshElemTypes,meshsize,abqC.HEX_DOMINATED,abqC.SYSTEM_ASSIGN)
bond_layers(DM,layer5, layer6)

# Update and add point marker for fixed faced boundary condition
FixedPoint[2]+=thickness
#layer6.FixedFace(DM,ApplyForceStep,(layer6.GetFaceFromPoint(DM,FixedPoint),))

# Create 7th layer
layer7 = Layer.CreateFromLayer(DM,layer6.gk_layer,delamo.CADwrap.OFFSET_DIRECTION,thickness,"Layer 7",LaminaSection,-45)
layer7.Finalize(DM)
layer7.MeshSimple(MeshElemTypes,meshsize,abqC.HEX_DOMINATED,abqC.SYSTEM_ASSIGN)
bond_layers(DM,layer6, layer7)

# Update and add point marker for fixed faced boundary condition
FixedPoint[2]+=thickness
FEModel.EncastreBC(name="FixedFace_%d" % (DM.get_unique()),
                   createStepName=ApplyForceStep.name,
                   region=layer7.singlepart.GetInstanceFaceRegion(FixedPoint,DM.pointtolerance))


# Create 8th layer
layer8 = Layer.CreateFromLayer(DM,layer7.gk_layer,delamo.CADwrap.OFFSET_DIRECTION,thickness,"Layer 8",LaminaSection,0)
layer8.Finalize(DM)
layer8.MeshSimple(MeshElemTypes,meshsize,abqC.HEX_DOMINATED,abqC.SYSTEM_ASSIGN)
bond_layers(DM,layer7, layer8)

# Update and add point marker for fixed faced boundary condition
FixedPoint[2]+=thickness
FEModel.EncastreBC(name="FixedFace_%d" % (DM.get_unique()),
                   createStepName=ApplyForceStep.name,
                   region=layer8.singlepart.GetInstanceFaceRegion(FixedPoint,DM.pointtolerance))



ForceVector=[ 0.0, 0.0, -5e-2 ] # Units of MPa 
# Call ABAQUS SurfaceTraction method
# Again, this came from looking at ABAQUS replay (.rpy) output
# Observe again that all ABAQUS symbolic constants need the "abqC"
# prefix. 
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

