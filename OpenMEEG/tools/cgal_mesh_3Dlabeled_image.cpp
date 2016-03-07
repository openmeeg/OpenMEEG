/*
Project Name : OpenMEEG

© INRIA and ENPC (contributors: Geoffray ADDE, Maureen CLERC, Alexandre
GRAMFORT, Renaud KERIVEN, Jan KYBIC, Perrine LANDREAU, Théodore PAPADOPOULO,
Emmanuel OLIVI
Maureen.Clerc.AT.sophia.inria.fr, keriven.AT.certis.enpc.fr,
kybic.AT.fel.cvut.cz, papadop.AT.sophia.inria.fr)

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
#include <options.h>
#include "cgal_mesh.h"
#include <CGAL/Labeled_image_mesh_domain_3.h>

using namespace OpenMEEG;
// To avoid verbose function and named parameters call
using namespace CGAL::parameters;

typedef CGAL::Labeled_image_mesh_domain_3<CGAL::Image_3,K> Mesh_domain;
typedef CGAL::Mesh_triangulation_3<Mesh_domain>::type Tr;
typedef CGAL::Mesh_complex_3_in_triangulation_3<Tr> C3t3;
// Criteria
typedef CGAL::Mesh_criteria_3<Tr> Mesh_criteria;

typedef Tr::Vertex_handle Vertex_handle;
typedef K::Point_3 Point_3;
typedef K::FT FT;

int main(int argc, char **argv) {
    command_usage("Create a BEM mesh from a 3D labeled image (e.g a mask):");
    const char * input_filename  = command_option("-i",(const char *) NULL,"Input image");
    const double radius_bound    = command_option("-fs",1e-1,"facet radius bound of elements");
    const double distance_bound  = command_option("-fd",1e-1,"facet distance bound to the input surface");
    const char * output_filename = command_option("-o",(const char *) NULL,"Output Mesh");

    if ( command_option("-h",(const char *)0,0) ) { 
        return 0; 
    }
    if ( output_filename == NULL ) {
        std::cerr << "Set an output filename" << std::endl;
        return 0;
    }

    // Mesh criteria
    CGAL::Image_3 image;
    image.read(input_filename);
    std::cout << "Input image:\n dimension: " << image.xdim() << "x"<< image.ydim() << "x"<< image.zdim() << std::endl;

    // Domain
    Mesh_domain domain(image, 0);

    // Mesh criteria
    Mesh_criteria criteria(facet_angle=30, facet_size=radius_bound, facet_distance=distance_bound);

    // Meshing
    C3t3 c3t3 = CGAL::make_mesh_3<C3t3>(domain, criteria, no_exude(), no_perturb());

    // Output
    Mesh m_out = CGAL_to_OM(c3t3);
    m_out.save(output_filename);
    m_out.info();
    return 0;
}
