/*
Project Name : OpenMEEG

© INRIA and ENPC (contributors: Geoffray ADDE, Maureen CLERC, Alexandre
GRAMFORT, Renaud KERIVEN, Jan KYBIC, Perrine LANDREAU, Théodore PAPADOPOULO,
Emmanuel OLIVI
Maureen.Clerc.AT.inria.fr, keriven.AT.certis.enpc.fr,
kybic.AT.fel.cvut.cz, papadop.AT.inria.fr)

The OpenMEEG software is a C++ package for solving the forward/inverse
problems of electroencephalography and magnetoencephalography.

This software is governed by the CeCILL-B license under French law and
abiding by the rules of distribution of free software.  You can  use,
modify and/ or redistribute the software under the terms of the CeCILL-B
license as circulated by CEA, CNRS and INRIA at the following URL
"http://www.cecill.info".

As a counterpart to the access to the source code and  rights to copy,
modify and redistribute granted by the license, users are provided only
with a limited warranty  and the software's authors,  the holders of the
economic rights,  and the successive licensors  have only  limited
liability.

In this respect, the user's attention is drawn to the risks associated
with loading,  using,  modifying and/or developing or reproducing the
software by the user in light of its specific status of free software,
that may mean  that it is complicated to manipulate,  and  that  also
therefore means  that it is reserved for developers  and  experienced
professionals having in-depth computer knowledge. Users are therefore
encouraged to load and test the software's suitability as regards their
requirements in conditions enabling the security of their systems and/or
data to be ensured and,  more generally, to use and operate it in the
same conditions as regards security.

The fact that you are presently reading this means that you have had
knowledge of the CeCILL-B license and that you accept its terms.
*/

#include <mesh.h>
#include "options.h"
#include <cgal_lib.h>

using namespace OpenMEEG;

int main(int argc, char **argv) {
    command_usage("Create a BEM mesh from either an implicit function: sphere, hemisphere, ...:");
    const double sphere_radius     = command_option("-r", 0.0, "radius of the sphere");
    const double hemisphere_radius = command_option("-hr",0.0, "radius of the hemisphere");
    const double radius_bound      = command_option("-fs",1e-1,"facet radius bound of elements");
    const double distance_bound    = command_option("-fd",1e-1,"facet distance bound to the input surface");
    // const unsigned init_points  = command_option("-ip", 10, "initial number of points (for the hemisphere)");
    const char * output_filename   = command_option("-o",nullptr,"Output Mesh");

    if (command_option("-h",nullptr,nullptr))
        return 0;

    if (output_filename==nullptr) {
        std::cerr << "Set an output filename" << std::endl;
        return 0;
    }

    Mesh m_out = cgal_mesh_function(sphere_radius,hemisphere_radius,radius_bound,distance_bound);
    m_out.save(output_filename);
    m_out.info();

    return 0;
}
