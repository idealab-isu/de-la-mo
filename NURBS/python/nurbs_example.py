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

import delamo.Support as sup

# Create a NURBS surface instance
surf = sup.NURBS()

# Set up the NURBS surface
surf.degree_u = 3
surf.degree_v = 3
surf.knotvector_u = [ 0, 0, 0, 0, 1, 2, 3, 3, 3, 3 ]
surf.knotvector_v = [ 0, 0, 0, 0, 1, 2, 3, 3, 3, 3 ]
# read_ctrlpts would automatically set weight to 1.0
surf.read_ctrlpts("data/NURBS_CP_Planar1.txt")

# Calculate surface points
surf.evaluate()

# Get the calculated surface point at (u, v) = (0.43, 0.84)
pt = surf.surfpt(0.43, 0.84)

# Calculate 2nd order derivatives at (u, v) = (0.3, 0.2)
skl = surf.derivatives(0.3, 0.2, 2)

# Calculate surface tangents and the normal at the given u, v position
tangent_u = surf.tangent_u(0.5, 0.6)
tangent_v = surf.tangent_v(0.5, 0.6)
norm = surf.normal(0.5, 0.6)
