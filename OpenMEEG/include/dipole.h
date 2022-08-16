// Project Name: OpenMEEG (http://openmeeg.github.io)
// © INRIA and ENPC under the French open source license CeCILL-B.
// See full copyright notice in the file LICENSE.txt
// If you make a copy of this file, you must either:
// - provide also LICENSE.txt and modify this header to refer to it.
// - replace this header by the LICENSE.txt content.

#pragma once

#include <matrix.h>
#include <vector.h>
#include <vect3.h>

namespace OpenMEEG {

    class OPENMEEG_EXPORT Dipole {
    public:

        Dipole(const Vector& V): r0(V(0),V(1),V(2)),q(V(3),V(4),V(5))   { }
        Dipole(const Vect3& pos,const Vect3& moment): r0(pos),q(moment) { }

        Dipole(const unsigned i,const Matrix& M): r0(M(i,0),M(i,1),M(i,2)),q(M(i,3),M(i,4),M(i,5)) { }

        double potential(const Vect3& r) const {

            // V = q.(r-r0)/||r-r0||^3

            const Vect3& x    = r-r0;
            const double nrm2 = x.norm2();
            return dotprod(q,x)/(nrm2*sqrt(nrm2));
        }

        const Vect3& position() const { return r0; }
        const Vect3& moment()   const { return q;  }

    private:

        const Vect3 r0;
        const Vect3 q;
    };
}
