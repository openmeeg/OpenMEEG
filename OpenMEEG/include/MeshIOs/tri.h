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

#pragma once

#include <iostream>
#include <string>

#include <om_utils.h>
#include <MeshIO.h>

namespace OpenMEEG::MeshIOs {

    /// \brief Mesh io for TRI file format.

    class OPENMEEG_EXPORT Tri: public MeshIO {

        typedef MeshIO base;

    public:

        void load_points(Geometry& geom) override {
            char ch;
            unsigned npts;
            fs >> ch >> npts;

            for (unsigned i=0; i<npts; ++i) {
                Vertex v;
                Normal n;
                fs >> v >> n;
                add_vertex(i,v,geom);
            }
        }

        void load_triangles(OpenMEEG::Mesh& mesh) override {
            reference_vertices(mesh);

            char ch;
            unsigned ntrgs;
            fs >> ch >> ntrgs >> ntrgs >> ntrgs; // This number is repeated 3 times

            mesh.triangles().reserve(ntrgs);
            for (unsigned i=0; i<ntrgs; ++i) {
                TriangleIndices t;
                fs >> t[0] >> t[1] >> t[2];
                add_triangle(t,mesh);
            }
        }

        void save(const OpenMEEG::Mesh& mesh,std::ostream& os) const override {
            os << "- " << mesh.vertices().size() << std::endl;

            const VertexIndices& vertex_index(mesh);
            for (const auto& vertex : mesh.vertices())
                os << *vertex << " " << mesh.normal(*vertex) << std::endl;

            const unsigned ntriangles = mesh.triangles().size();
            os << "- " << ntriangles << ' ' << ntriangles << ' ' << ntriangles << std::endl;
            for (const auto& triangle : mesh.triangles())
                os << vertex_index(triangle,0) << ' '
                   << vertex_index(triangle,1) << ' '
                   << vertex_index(triangle,2) << std::endl;
        }

    private:

        MeshIO* clone(const std::string& filename) const override { return new Tri(filename); }

        Tri(const std::string& filename=""): base(filename,"tri") { }

        static const Tri prototype;

        const char* name() const override { return "TRI"; }
    };
}
