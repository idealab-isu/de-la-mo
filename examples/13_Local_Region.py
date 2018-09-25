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
 layer_boundary_template) = process.output_filenames("13_Local_Region",phase="ORIGINAL",process=None)


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
FEModelMeshing=DM.meshinstrs.rewrapobj(FEModel)
# Creates a reference to the FEModel, for which method calls
# execute in the meshing context



# This script then generates both a CAD file and a Python script.
# The Python script can be run from Abaqus. It includes the
# initialization script referenced above, and also opens
# the CAD file and builds the model.


# Basic parameters
# Set layer thickness
thickness = 0.18288 # mm

# Create 1st layer from SAT file mold
                        
layer1 = Layer.CreateFromSAT(DM,"data/13_NASA_Local_Mold.SAT",delamo.CADwrap.OFFSET_DIRECTION,thickness,"Layer 1 ", LaminaSection,0)
layer1.Finalize(DM)
layer1.MeshSimple(MeshElemTypes,meshsize,abqC.HEX_DOMINATED,abqC.SYSTEM_ASSIGN)




# Create 2nd layer

layer2 = Layer.CreateFromLayer(DM,layer1.gk_layer,delamo.CADwrap.OFFSET_DIRECTION,thickness,"Layer 2 ", LaminaSection,90)
layer2.Finalize(DM)
layer2.MeshSimple(MeshElemTypes,meshsize,abqC.HEX_DOMINATED,abqC.SYSTEM_ASSIGN)

#Bond Layer 1 and 2
bond_layers(DM, layer1, layer2)

# Create 3rd layer -- should use finemeshsize because this is where
layer3 = Layer.CreateFromLayer(DM,layer2.gk_layer,delamo.CADwrap.OFFSET_DIRECTION,thickness,"Layer 3 ",LaminaSection,0)
layer3.Finalize(DM)
layer3.MeshSimple(MeshElemTypes,meshsize,abqC.HEX_DOMINATED,abqC.SYSTEM_ASSIGN)


bond_layers(DM, layer2, layer3, defaultBC=delamo.CADwrap.BC_COHESIVE,
            CohesiveInteraction=CohesiveInteraction,
            ContactInteraction=ContactInteraction,
            delaminationlist = [ "data/nasa-local-delam-1.csv",
                                 "data/nasa-local-delam-2.csv" ])
#bond_layers(DM, layer2, layer3)

# Create 4th layer -- should use finemeshsize because this is where
# delamination might go
layer4 = Layer.CreateFromLayer(DM,layer3.gk_layer,delamo.CADwrap.OFFSET_DIRECTION,thickness,"Layer 4 ",LaminaSection,90)
layer4.Finalize(DM)
layer4.MeshSimple(MeshElemTypes,meshsize,abqC.HEX_DOMINATED,abqC.SYSTEM_ASSIGN)

bond_layers(DM, layer3, layer4)


# You can have the job auto-start when the Python script is run
#DM.RunJob(BendingJob)

DM.Finalize(script_to_generate,cad_file_path_from_script)
