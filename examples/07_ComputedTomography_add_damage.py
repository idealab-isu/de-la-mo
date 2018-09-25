# This script copies the damage data into the active delamo
# output directory for 07_ComputedTomography_Damage.py

import os
import os.path
import glob
import re
import shutil

output_directory = "07_ComputedTomography_Damage_output/"

boundary_layers = glob.glob(os.path.join(output_directory,"layerboundary_PREDAMAGE_*.stl"))
if len(boundary_layers) != 7:
    raise ValueError("Did not find exactly seven PREDAMAGE .stl files in %s" % (output_directory))

linenumbers = [ int(re.match(r"""layerboundary_PREDAMAGE_([0-9]+)[.]stl""",os.path.split(boundary_layer_filename)[1]).group(1)) for boundary_layer_filename in boundary_layers ]
linenumbers.sort()

# At the first boundary, index 0 (layers 1-2) have two delaminations
shutil.copyfile("data/nasa-delam12-1.csv",os.path.join(output_directory,"layerdelam_%5.5d_1.csv" % (linenumbers[0])))
shutil.copyfile("data/nasa-delam12-2.csv",os.path.join(output_directory,"layerdelam_%5.5d_2.csv" % (linenumbers[0])))

# At the second boundary, index 1(layers 2-3), also have two delaminations
shutil.copyfile("data/nasa-delam23-1.csv",os.path.join(output_directory,"layerdelam_%5.5d_1.csv" % (linenumbers[1])))
shutil.copyfile("data/nasa-delam23-2.csv",os.path.join(output_directory,"layerdelam_%5.5d_2.csv" % (linenumbers[1])))

# At the third boundary, index 2(layers 3-4), also have two delaminations
shutil.copyfile("data/nasa-delam34-1.csv",os.path.join(output_directory,"layerdelam_%5.5d_1.csv" % (linenumbers[2])))
shutil.copyfile("data/nasa-delam34-2.csv",os.path.join(output_directory,"layerdelam_%5.5d_2.csv" % (linenumbers[2])))

# At the fourth boundary, index 3(layers 4-5), have no delaminations

# At the fifth  boundary, index 4(layers 5-6), have two delaminations
shutil.copyfile("data/nasa-delam56-1.csv",os.path.join(output_directory,"layerdelam_%5.5d_1.csv" % (linenumbers[4])))
shutil.copyfile("data/nasa-delam56-2.csv",os.path.join(output_directory,"layerdelam_%5.5d_2.csv" % (linenumbers[4])))

# At the sixth boundary, index 5 (layers 6-7), have two delaminations
shutil.copyfile("data/nasa-delam67-1.csv",os.path.join(output_directory,"layerdelam_%5.5d_1.csv" % (linenumbers[5])))
shutil.copyfile("data/nasa-delam67-2.csv",os.path.join(output_directory,"layerdelam_%5.5d_2.csv" % (linenumbers[5])))
