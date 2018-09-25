import os
import sys
import re
sys.path.append(os.getcwd())

import delamo.CADmodeler as cadmb
import delamo.CADsupport as CADsupport

BC_COHESIVE = cadmb.is_cohesive
BC_CONTACT = cadmb.is_contact
BC_TIE = cadmb.is_tie
BC_NONE = cadmb.is_none
BC_COHESIVE_LAYER = cadmb.is_cohesive_layer
Delamination_COHESIVE = cadmb.COHESIVE
Delamination_CONTACT = cadmb.CONTACT
Delamination_NOMODEL = cadmb.NOMODEL
Delamination_TIE = cadmb.TIE
OFFSET_DIRECTION = cadmb.OFFSET
ORIG_DIRECTION = cadmb.ORIG

# Necessary wrappers for Swig-wrapped C++ libraries
# @see: http://stackoverflow.com/questions/4975509/lifetime-of-temporary-objects-in-swigs-python-wrappers


# NURBS class wrapper
class NURBSd(CADsupport.NURBSd):
    last_obj = None

    def __init__(self):
        super(NURBSd, self).__init__()
        NURBSd.last_obj = self


class TPoint3d(CADsupport.TPoint3d):
    last_obj = None

    def __init__(self):
        super(TPoint3d, self).__init__()
        TPoint3d.last_obj = self


#class Vector3(CADsupport.Vector3):
#    last_obj = None
#
#    def __init__(self):
#        super(Vector3, self).__init__()
#        Vector3.last_obj = self


# delamo::List< std:: string >
class StringList(cadmb.StringList):
    last_obj = None
    
    def __init__(self):
        super(StringList, self).__init__()
        StringList.last_obj = self


# delamo::List<FaceAdjacency>
class FAL(cadmb.FAL):
    last_obj = None

    def __init__(self):
        super(FAL, self).__init__()
        FAL.last_obj = self


# delamo::List< Layer >
class LayerList(cadmb.LayerList):
    last_obj = None
    
    def __init__(self):
        super(LayerList, self).__init__()
        LayerList.last_obj = self

class MBBodyList(cadmb.MBBodyList):
    last_obj = None
    
    def __init__(self):
        super(MBBodyList, self).__init__()
        MBBodyList.last_obj = self
        
class TPoint3dList(CADsupport.TPoint3dList):
    last_obj = None
    
    def __init__(self):
        super(TPoint3dList, self).__init__()
        TPoint3dList.last_obj = self






# delamo::List< LayerMold >
class LayerMoldList(cadmb.LayerMoldList):
    last_obj = None

    def __init__(self):
        super(LayerMoldList, self).__init__()
        LayerMoldList.last_obj = self


# Layer class wrapper
class Layer(cadmb.Layer):
    last_obj = None

    def __init__(self):
        super(Layer, self).__init__()
        Layer.last_obj = self


# LayerBody class wrapper
class LayerBody(cadmb.LayerBody):
    last_obj = None

    def __init__(self):
        super(LayerBody, self).__init__()
        LayerBody.last_obj = self


# LayerSurface class wrapper
class LayerSurface(cadmb.LayerSurface):
    last_obj = None

    def __init__(self):
        super(LayerSurface, self).__init__()
        LayerSurface.last_obj = self


# LayerMold class wrapper
class LayerMold(cadmb.LayerMold):
    last_obj = None

    def __init__(self):
        super(LayerMold, self).__init__()
        LayerMold.last_obj = self


# ACISModelBuilder class wrapper with additional functionality
class ModelBuilder(cadmb.ACISModelBuilder):
    last_obj = None

    def __init__(self, **kwargs):
        unlock_str = kwargs.get('license_key', "")
        super(ModelBuilder, self).__init__(unlock_str)
        ModelBuilder.last_obj = self
        self.start()

def read_license_key(filename):
    """ Reads ACIS license key from a text file.
    
    Function retrieved from https://github.com/orbingol/ACIS-Python3
    
    :param filename: file containing the unlock key
    :type filename: str
    :return: the unlock key
    """
    regex_seq = re.compile(r'\s?\"(.*)\"')

    license_text = ""
    search_obj = None

    try:
        with open(filename, 'r') as fp:
            license_code = fp.read()
            search_obj = regex_seq.finditer(license_code.strip())
            fp.close()
    except IOError:
        raise IOError('Cannot open file: ' + filename)

    if search_obj:
        for line in search_obj:
            license_text = license_text + line.group(1)
    else:
        raise ValueError('The file "' + filename + '" does not contain the unlock key!')

    return license_text
