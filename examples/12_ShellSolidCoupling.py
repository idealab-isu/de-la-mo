import numpy as np
import delamo.CADwrap
from delamo.api import DelamoModeler
from delamo.api import Part 
from delamo.api import Assembly
from delamo.api import Layer
from delamo.api import bond_layers
from delamo.api import shell_and_cutout_from_shelltool
from delamo.api import shell_solid_coupling
from delamo import process

import os

# Front matters
# ------------

# Read ACIS license key
acis_license = delamo.CADwrap.read_license_key(filename="license.dat")

# Initialize the DeLaMo model
DM=DelamoModeler.Initialize(globals(),
                            pointtolerancefactor=3.0,
                            normaltolerance=100e-4,
                            tangenttolerance=100e-4,
                            license_key=acis_license)

# CAD file to write and script file to generate. 
# The first parameter to output_filenames
# should always match the name of the original script
# with the ".py" stripped
(script_to_generate,
 cad_file_path_from_script,
 layer_boundary_template) = process.output_filenames("12_ShellSolidCoupling",phase="ORIGINAL",process=None)

# When writing a DeLaMo script, you start by creating a 
# finite element initialization script. This is a 
# Python script for ABAQUS that defines your various parameters
# -- material properties, etc. as Python variables.  
# In this case they are stored in the "abqparams_CFRP.py" and "abqparams_CFRP_cohesive.py" files
DM.abaqus_init_script("abqparams_CFRP.py",globals())
DM.abaqus_init_script("abqparams_CFRP_cohesive.py",globals())

# Coarsen mesh for speed
meshsize=3.0 # mm

# The above call automatically inserts wrapped copies of variables 
# defined in those scripts into the global variable space. Then you 
# can reference those variables in this script 


# The DM contains generates several sets of instructions
# for different phases of the finite element modeling process:
# DM.initinstrs  (initialization)
# DM.assemblyinstrs (model assembly)
# DM.bcinstrs (boundary conditions)
# DM.meshinstrs (meshing)

# All methods called from those variables will generally be executed
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

# Perform the cutout operation
# WARNING: Names of shell and cutout are hardwired in ModelBuilder so it will never work to do this more than once!
(shell,cutout,edgeinfo) = delamo.api.shell_and_cutout_from_shelltool(DM,"data/FlatShellTool.SAT")



# layer_mold_list[1], which is bigger piece with hole in it
# DM.modelbuilder.load_shell_model("data/FlatShell2.SAT", shell_edge_point_list, shell_edge_tangent_list, shell_edge_normal_list)

## Need to be able to save layer_mold_list[1] into .sat file and pull it out from Abaqus

## WARNING: call to openAcis must be immediately before PartFromGeometryFile()
## Because of apparent Abaqus bug whereby it confuses differently loaded geometry  files.  (Only one open at a time?)
## That is why we put the openAcis() in assemblyinstrs
#abq_assemblyinstrs=DM.assemblyinstrs.preexisting_variable("abq")
#shellgeomfile=abq_assemblyinstrs.mdb.openAcis("data/FlatShell2.SAT",scaleFromFile=abqC.ON) # abq variable reference runs this command in in initstrs
#shellgeom_name="Shell"
#shellgeom_bodynum=1
#shell_geom_fe_part=FEModel.PartFromGeometryFile(name=shellgeom_name,
#                                                                  geometryFile=shellgeomfile,
#                                                                  bodyNum=shellgeom_bodynum,
#                                                                  combine=False,
#                                                                  dimensionality=abqC.THREE_D,
#                                                                  type=abqC.DEFORMABLE_BODY)
#
#ShellGeomPart=Part(DM=DM,fe_part=shell_geom_fe_part,name=shellgeom_name,shell=True)

ShellMat=FEModel.Material(name="ShellMat")
ShellMat.Elastic(table=((200e9/1.e6,0.22),))

ShellSection=FEModel.HomogeneousShellSection(name="ShellSect",
                                             preIntegrate=abqC.OFF,
                                             material="ShellMat",
                                             thicknessType=abqC.UNIFORM,
                                             thickness=3.0,
                                             thicknessField='',
                                             idealization=abqC.NO_IDEALIZATION,
                                             poissonDefinition=abqC.DEFAULT,
                                             thicknessModulus=None,
                                             temperature=abqC.GRADIENT,
                                             useDensity=abqC.OFF,
                                             integrationRule=abqC.SIMPSON,
                                             numIntPts=5)
shell.AssignSection(ShellSection)
#ShellGeomPart.CreateInstance(dependent=abqC.ON)


ShellMeshElemTypes=(mesh.ElemType(elemCode=abqC.S8R, elemLibrary=abqC.STANDARD),
                    mesh.ElemType(elemCode=abqC.STRI65, elemLibrary=abqC.STANDARD))
shellmeshsize=100.0

#shell_geom_meshing = DM.meshinstrs.rewrapobj(ShellGeomPart)
#shell_refined_edges = [ (shell_edge_point_list[edgecnt],shell_edge_tangent_list[edgecnt]) for edgecnt in range(len(shell_edge_point_list)) ] 
#shell_refined_edges=[]
shell.MeshSimple(ShellMeshElemTypes,shellmeshsize,abqC.QUAD_DOMINATED,abqC.SYSTEM_ASSIGN,refinedmeshsize=12.0,refined_edges=edgeinfo,pointtolerance=DM.pointtolerance,tangenttolerance=DM.normaltolerance)


orientation=[0,45,-45,90,90,-45,45,0]

#numlayers=8
numlayers=4
prev_layer=None

ssc = shell_solid_coupling.from_shell_and_cutout(DM,shell,cutout)

for layernum in range(numlayers):
    # Create layer

    if layernum==0:
        # layer_mold_list[0] is little tiny piece, to be bonded into...
        #layer = Layer.CreateFromSAT(DM,"data/FlatMold2.SAT",delamo.CADwrap.OFFSET_DIRECTION,thickness,"Layer %d" % (layernum),LaminaSection,orientation[layernum])
        layer = Layer.CreateFromMold(DM,cutout.gk_part,delamo.CADwrap.OFFSET_DIRECTION,thickness,"Layer %d" % (layernum),LaminaSection,orientation[layernum])
        pass
    else:
        layer = Layer.CreateFromLayer(DM,prev_layer.gk_layer,delamo.CADwrap.OFFSET_DIRECTION,thickness,"Layer %d" % (layernum), LaminaSection,orientation[layernum])
        pass



    layer.Finalize(DM)
    
    layeroffset=thickness + (layernum+0.5)*thickness  # !!!*** Why did we need to add in the extra initial thickness (?) Probably a bug in the modelbuilder ... The Mold_Region object shows in the correct position
    # ... Seems to definitely be a modelbuilder bug, where the layer is shifted up by a thickness  (Mold treated as if it has thickness) but
    # when it goes to find face points/normals it gets them where the layer should be, not where it is!!!
    # Temporary workaround: ABQ can only handle a single set of SSC's around the boundary .. Should really do this for center layer. 

    #import pdb
    #pdb.set_trace()
    ssc.bond_layer(DM,layer,layeroffset)   
    



    
    layer.MeshSimple(MeshElemTypes,meshsize,abqC.HEX_DOMINATED,abqC.SYSTEM_ASSIGN)

    if prev_layer is not None:
        bond_layers(DM,prev_layer,layer,
                    defaultBC=delamo.CADwrap.Delamination_COHESIVE,
                    CohesiveInteraction=CohesiveInteraction,
                    ContactInteraction=ContactInteraction) 
        pass

    prev_layer=layer

    pass

ssc.Finalize(DM,influenceDistance=6.0)
#solidfaces2=DM.GetFaces(layer1.fe_layer.parts[0].instance.faces,(((0,50,thickness/2.0),layer1SideFaceNormalList[2]),),DM.facepointtolerance,DM.normaltolerance)
#SolidFaceRegion2=regionToolset.Region(side1Faces=solidfaces2)
#SolidSurf=Laminate.Surface(name="SolidSurf",side1Faces=solidfaces)

#ShellGeomPartBC=DM.bcinstrs.rewrapobj(ShellGeomPart)
# Either need to modify edge algorithm or get tangent... !!!*** (replace 1.0,0.0)
#shelledges2=DM.GetEdges(ShellGeomPart.instance.edges,ShellGeomPart.abq_part.vertices,((tuple(shell_edge_point_list[2]),(1.0,0.,0.)),),0.1,DM.normaltolerance)
#ShellRegion2=regionToolset.Region(side1Edges=shelledges2)

#ShellSurf=Laminate.Surface(name="ShellSurf",side1Edges=shelledges)


#FEModel.ShellSolidCoupling(name='ssc2',
#                                 shellEdge=ShellRegion2,
#                                 solidFace=SolidFaceRegion2,
#                                 positionToleranceMethod=abqC.COMPUTED,
#                                 influenceDistanceMethod=abqC.DEFAULT)






#shellfixededges=DM.GetEdges(ShellGeomPart.instance.edges,ShellGeomPart.abq_part.vertices,(((-500.0,-500.0,0.0),(0.0,1.,0.)),),DM.facepointtolerance,DM.normaltolerance)
#ShellRegion=regionToolset.Region(side1Edges=shelledges)
#DM.BoundaryCondition.FixedEdge(FEModel,ShellGeomParts,"EncastreBC_%d" % (DM.get_unique()),None,ApplyForceStep,(((-500.0,0.0,0.0),(0.0,1.,0.)),),DM.facepointtolerance,DM.normaltolerance)

FEModel.EncastreBC(name="EncastreBC_%d" % (DM.get_unique()),
                   createStepName=ApplyForceStep.name,
                   region=shell.GetInstanceEdgeRegion((-500.0,0.0,0.0),DM.pointtolerance))

#DM.BoundaryCondition.FixedEdge(FEModel,ShellGeomParts,"EncastreBC_%d" % (DM.get_unique()),None,ApplyForceStep,(((500.0,0.0,0.0),(0.0,1.,0.)),),DM.facepointtolerance,DM.normaltolerance)
FEModel.EncastreBC(name="EncastreBC_%d" % (DM.get_unique()),
                   createStepName=ApplyForceStep.name,
                   region=shell.GetInstanceEdgeRegion((500.0,0.0,0.0),DM.pointtolerance))


                                 

ForceVector=[ 0.0, 0.0, -5e-2 ] # Units of MPa 

# Apply surface traction to final layer
#layer.SurfaceTraction(DM,ApplyForceStep,(layer.GetFaceFromPoint(DM,(24.0,24.0,thickness*numlayers)),),ForceVector,name="AppliedTraction")
FEModel.SurfaceTraction(name="SurfaceTraction%d" % (DM.get_unique()),
                        createStepName=ApplyForceStep.name,
                        region=layer.singlepart.GetInstanceFaceRegionSurface((24.0,24.0,thickness*numlayers),DM.pointtolerance),
                        distributionType=abqC.UNIFORM,
                        field='',
                        localCsys=None,
                        traction=abqC.GENERAL,
                        follower=abqC.OFF,
                        resultant=abqC.ON,
                        magnitude=np.linalg.norm(ForceVector),
                        directionVector=( (0,0,0), tuple(ForceVector/np.linalg.norm(ForceVector)) ),
                        amplitude=abqC.UNSET)



DM.Finalize(script_to_generate,cad_file_path_from_script)

# You can have the job auto-start when the Python script is run
#DM.RunJob(BendingJob)
