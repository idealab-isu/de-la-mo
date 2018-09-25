# Copyright (c) 2018 by Adarsh Krishnamurthy et. al. and Iowa State University. 
# All rights reserved.
#
# Permission to use, copy, modify, and distribute this software and its
# documentation for non-profit use, without fee, and without written agreement is
# hereby granted, provided that the above copyright notice and the following
# two paragraphs appear in all copies of this software.
#
# IN NO EVENT SHALL IOWA STATE UNIVERSITY BE LIABLE TO ANY PARTY FOR
# DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
# OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF IOWA STATE UNIVERSITY
# HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
# IOWA STATE UNIVERSITY SPECIFICALLY DISCLAIMS ANY WARRANTIES,
# INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
# AND FITNESS FOR A PARTICULAR PURPOSE.  THE SOFTWARE PROVIDED HEREUNDER IS
# ON AN "AS IS" BASIS, AND IOWA STATE UNIVERSITY HAS NO OBLIGATION TO
# PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
#
#
# Initial version April 20 2018 - Adarsh Krishnamurthy et. al.
#

import os
import sys
sys.path.append(os.getcwd())

import delamo.CADsupport as sup

# Necessary wrappers for Swig-wrapped C++ libraries
# @see: http://stackoverflow.com/questions/4975509/lifetime-of-temporary-objects-in-swigs-python-wrappers



# NURBS class wrapper 
# NOTE: in delamo, this is duplicated in CADwrap.py
class NURBSd(sup.NURBSd):
    last_obj = None

    def __init__(self):
        super(NURBSd, self).__init__()
        NURBSd.last_obj = self
