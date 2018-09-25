#
# ACIS Example 01
#

# Import ModelBuilder library
import os
import delamo.CADwrap

# Fix file path
os.chdir(os.path.dirname(os.path.realpath(__file__)))


# Set layer thickness
thickness = 0.25
cad_file_name = "ACIS_Example01.sat"

# Create the NURBS mold surface
mold = delamo.CADwrap.NURBSd()
mold.degree_u = 3
mold.degree_v = 3
mold.knotvector_u = [ 0, 0, 0, 0, 1, 2, 3, 3, 3, 3 ]
mold.knotvector_v = [ 0, 0, 0, 0, 1, 2, 3, 3, 3, 3 ]
# read_ctrlpts would automatically set weight to 1.0
mold.read_ctrlpts("data/CP_Planar1.txt")

# Read license
acis_license = delamo.CADwrap.read_license_key("license.dat")

# Initialize the ACIS Model Builder
acis = delamo.CADwrap.ModelBuilder(license_key=acis_license)

# Create 1st layer from the NURBS surface
layer1 = delamo.CADwrap.Layer()
acis.create_layer(mold, delamo.CADwrap.OFFSET_DIRECTION, thickness, layer1)
layer1.name("Layer_1")
layer1.layup(0)
layer1.id(1)

# Create 2nd layer from the 1st layer
layer2 = delamo.CADwrap.Layer()
acis.create_layer(layer1, delamo.CADwrap.OFFSET_DIRECTION, thickness, layer2)
layer2.name("Layer_2")
layer2.layup(90)
layer2.id(2)

# Bond Layers 1 and 2
list12 = acis.adjacent_layers(layer1, layer2, delamo.CADwrap.BC_DEFAULT_CONTACT)

# Create 3rd layer
layer3 = delamo.CADwrap.Layer()
acis.create_layer(layer2, delamo.CADwrap.OFFSET_DIRECTION, thickness, layer3)
layer3.name("Layer_3")
layer3.layup(0)
layer3.id(3)

# Bond Layers 2 and 3
list23 = acis.adjacent_layers(layer2, layer3, delamo.CADwrap.BC_DEFAULT_CONTACT)

# Create 4th layer
layer4 = delamo.CADwrap.Layer()
acis.create_layer(layer3, delamo.CADwrap.OFFSET_DIRECTION, thickness, layer4)
layer4.name("Layer_4")
layer4.layup(90)
layer4.id(4)

# Bond Layers 3 and 4
list34 = acis.adjacent_layers(layer3, layer4, "data/Delamination1_3D.csv", delamo.CADwrap.BC_DEFAULT_CONTACT)

# Create 5th layer
layer5 = delamo.CADwrap.Layer()
acis.create_layer(layer4, delamo.CADwrap.OFFSET_DIRECTION, thickness, layer5)
layer5.name("Layer 5")
layer5.layup(90)
layer5.id(5)

# Bond Layers 4 and 5
list45 = acis.adjacent_layers(layer4, layer5, delamo.CADwrap.BC_DEFAULT_CONTACT)

# Create 6th layer
layer6 = delamo.CADwrap.Layer()
acis.create_layer(layer5, delamo.CADwrap.OFFSET_DIRECTION, thickness, layer6)
layer6.name("Layer_6")
layer6.layup(0)
layer6.id(6)

# Bond Layers 5 and 6
list56 = acis.adjacent_layers(layer5, layer6, delamo.CADwrap.BC_DEFAULT_CONTACT)

# Create 7th layer
layer7 = delamo.CADwrap.Layer()
acis.create_layer(layer6, delamo.CADwrap.OFFSET_DIRECTION, thickness, layer7)
layer7.name("Layer_7")
layer7.layup(90)
layer7.id(7)

# Bond Layers 6 and 7
list67 = acis.adjacent_layers(layer6, layer7, delamo.CADwrap.BC_DEFAULT_CONTACT)

# Create 8th layer
layer8 = delamo.CADwrap.Layer()
acis.create_layer(layer7, delamo.CADwrap.OFFSET_DIRECTION, thickness, layer8)
layer8.name("Layer_8")
layer8.layup(0)
layer8.id(8)

# Bond Layers 7 and 8
list78 = acis.adjacent_layers(layer7, layer8, delamo.CADwrap.BC_DEFAULT_CONTACT)

l7_lbnameslist = layer7.bodynames(0)

# Save SAT file using a std::string container
# bodynames_list = delamo.CADwrap.StringList()
# acis.save(cad_file_name, body_names)
# for i in range(len(body_names)):
#     print(body_names[i])

# List of layers to be saved
layer_list = delamo.CADwrap.LayerList()
layer_list.append(layer1)
layer_list.append(layer2)
layer_list.append(layer3)
layer_list.append(layer4)
layer_list.append(layer5)
layer_list.append(layer6)
layer_list.append(layer7)
layer_list.append(layer8)

# List of body names
names_list = delamo.CADwrap.StringList()

# Save SAT file
#bodynames_list = acis.save(cad_file_name);
acis.save(cad_file_name, layer_list, names_list)


fixed_point = [-30.0, -24.0, 0.1]

# Get fixed BC for all layers
point, normal, name = acis.find_closest_face_to_point(layer1, fixed_point)

# Stop ModelBuilder
acis.stop()

# This is here to put a breakpoint
print("End of ACIS Model Builder Code")
