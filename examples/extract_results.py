from visualization import *
from odbAccess import openOdb
import abaqusConstants as abqC
import numpy as np


# Input odb files to plot
odbfiles=['/home/bryans70/de-la-mo/install/examples/04_Planar_Single_Delam.odb', '/home/bryans70/de-la-mo/install/examples/06_Planar_No_Delam.odb']

load_deflection=[] # List of tuples: [(load1, deflection1), (load2, deflection2)...]
# load1, deflection1, etc. are lists of load and deflection for each frame in the odb

extract_load = 'ForceSurface' # Surface over which to extract load
extract_disp = 'DispNode' # Node set to extract displacement from
forcestep = 'ApplyForceStep' # Load step

for odbfile in odbfiles:
    print "Opening database {f}".format(f=odbfile)
    odb = openOdb(path=odbfile)

    load_instances=[]
    disp_instances=[]

    surface = extract_load.upper()
    node = extract_disp.upper()

    for key, instance in odb.rootAssembly.instances.items():
        if instance.surfaces.has_key(surface):
            # Find the instance which has the loading surface
            load_el_set = odb.rootAssembly.instances[instance.name].surfaces[surface].elements # Sequence of element sets, one for each instance spanned by the         
            load_instance=instance
        if instance.nodeSets.has_key(node):
            # Find the instance which has the node set
            disp_node_set = odb.rootAssembly.instances[instance.name].nodeSets[node].nodes # Sequence of node sets, one for each instance spanned by the node set
element set
            disp_instance=instance
        else:
            pass

    # All frames in ODB
    frames = odb.steps[forcestep].frames

    el_vals_list = []
    node_vals_list = []

    framecnt=0
    for frame in frames:
        print "Frame: {f}".format(f=framecnt)
        loads=frame.fieldOutputs['P']
        disps=frame.fieldOutputs['U']
        
        load_vals = loads.values
        disp_vals = disps.values

        el_vals = []
        node_vals = []

        # For pressure (scalar field) 
        for idx in range(len(load_el_set)):
            el load_el_set[idx]
            i = el.label
           
            # For some reason, using getSubset is not finding the correct values
            # This is a 'hack' to get the correct index
            # We can at least ensure that we are referencing the correct element
            assert( load_vals[idx].elementLabel == el.label )
            el_vals.append(load_vals[idx].data) 
          
            #el_vals.append(loads.getSubset(region=load_instance.getNodeFromLabel(i), position=abqC.ELEMENT_FACE)) # Should work, but doesn't
        
        el_vals_list.append(el_vals)

        for node in disp_node_set:
            i = node.label
            sub = disps.getSubset(region=disp_instance.getNodeFromLabel(i))
            node_vals.append(sub.values[0].magnitude)

        node_vals_list.append(node_vals)

        framecnt+=1
        pass
        
    deflection = np.mean(np.array(node_vals_list), axis=1)
    load = np.abs(np.mean(np.array(el_vals_list), axis=1))

    load_deflection.append((load, deflection))

    odb.close()


# Create plot to view load-displacement curve

xyview = session.Viewport(name='CustomView')
xyview.maximize()
xyplot = session.XYPlot(name='DeflectionPlot')

xyview.setValues(displayedObject=xyplot)

odbcnt = 0
for load, deflection in load_deflection:

    print "Number of Frames: {n}".format(n=len(load))
    data = []
    for idx in range(len(load)):
        data.append((load[idx], np.abs(deflection[idx])))

    xydata = session.XYData(name=odbfiles[odbcnt][:-4], data=data)
    xycurve = session.Curve(odbfiles[odbcnt][:-4], xydata)

    odbcnt += 1

xyplot.charts.items()[0][1].setValues(curvesToPlot=[odbfiles[i][:-4] for i in range(0,odbcnt)])





