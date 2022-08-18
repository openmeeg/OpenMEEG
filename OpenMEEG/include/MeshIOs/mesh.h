// Project Name: OpenMEEG (http://openmeeg.github.io)
// © INRIA and ENPC under the French open source license CeCILL-B.
// See full copyright notice in the file LICENSE.txt
// If you make a copy of this file, you must either:
// - provide also LICENSE.txt and modify this header to refer to it.
// - replace this header by the LICENSE.txt content.

#pragma once

#include <iostream>
#include <fstream>

#include <map>
#include <string>

#include <om_utils.h>
#include <MeshIO.h>

namespace OpenMEEG::MeshIOs {

    /// \brief Mesh io for TRI file format.

    class OPENMEEG_EXPORT Mesh: public MeshIO {

        typedef MeshIO base;

    public:

        void load_points(Geometry& geom) override {

            unsigned char uc[5];
            fs.read(reinterpret_cast<char*>(uc),5); // File format
            fs.read(reinterpret_cast<char*>(uc),4); // little/big endian
            //  TODO: we should check that these values are correct.

            unsigned arg_size;
            fs.read(reinterpret_cast<char*>(&arg_size),sizeof(unsigned)); // Should be 4
            fs.read(reinterpret_cast<char*>(&arg_size),arg_size); // Should be characters VOID.

            unsigned vertex_per_face;
            fs.read(reinterpret_cast<char*>(&vertex_per_face),sizeof(unsigned));

            unsigned mesh_time;
            fs.read(reinterpret_cast<char*>(&mesh_time),sizeof(unsigned));
            fs.ignore(sizeof(unsigned)); // mesh_step

            // Support only for triangulations and one time frame.

            if (vertex_per_face!=3)
                throw std::invalid_argument("OpenMEEG only handles 3D surfacic meshes.");

            if (mesh_time!=1)
                throw std::invalid_argument("OpenMEEG only handles 3D surfacic meshes with one time frame.");

            unsigned npts;
            fs.read(reinterpret_cast<char*>(&npts),sizeof(unsigned));

            float* coords = new float[3*npts]; // Point coordinates
            fs.read(reinterpret_cast<char*>(coords),3*npts*sizeof(float));
            Vertices vertices;
            for (unsigned i=0,j=0; i<npts; ++i,j+=3)
                vertices.push_back(Vertex(coords[j],coords[j+1],coords[j+2]));
            indmap = geom.add_vertices(vertices);
            delete[] coords;

            fs.read(reinterpret_cast<char*>(&npts),sizeof(unsigned)); // Number of normals
            fs.ignore(3*npts*sizeof(float)); // Ignore normals.
            fs.ignore(sizeof(unsigned));
        }

        void load_triangles(OpenMEEG::Mesh& mesh) override {
            reference_vertices(mesh);

            unsigned ntrgs; // Number of faces
            fs.read(reinterpret_cast<char*>(&ntrgs),sizeof(unsigned));

            unsigned* pts_inds = new unsigned[3*ntrgs]; // Faces
            fs.read(reinterpret_cast<char*>(pts_inds),3*ntrgs*sizeof(unsigned));
            mesh.triangles().reserve(ntrgs);
            for (unsigned i=0,j=0; i<ntrgs; ++i,j+=3) {
                const TriangleIndices t = { pts_inds[j], pts_inds[j+1], pts_inds[j+2] };
                mesh.add_triangle(t,indmap);
            }
            delete[] pts_inds;
        }

        void save(const OpenMEEG::Mesh& mesh,std::ostream& os) const override {
            unsigned char format[5] = {'b', 'i', 'n', 'a', 'r'}; // File format
            os.write(reinterpret_cast<char*>(format),5);

            unsigned char lbendian[4] = {'D', 'C', 'B', 'A'}; // little/big endian
            os.write(reinterpret_cast<char*>(lbendian),4);

            unsigned arg_size = 4;
            os.write(reinterpret_cast<char*>(&arg_size),sizeof(unsigned));

            unsigned char VOID[4] = {'V', 'O', 'I', 'D'}; // Trash
            os.write(reinterpret_cast<char*>(VOID),4);

            unsigned vertex_per_face = 3;
            os.write(reinterpret_cast<char*>(&vertex_per_face),sizeof(unsigned));

            unsigned mesh_time = 1;
            os.write(reinterpret_cast<char*>(&mesh_time),sizeof(unsigned));

            unsigned mesh_step = 0;
            os.write(reinterpret_cast<char*>(&mesh_step),sizeof(unsigned));

            //  Vertices

            float* pts_raw     = new float[mesh.vertices().size()*3]; // Points
            float* normals_raw = new float[mesh.vertices().size()*3]; // Normals

            const VertexIndices& vertex_index(mesh);

            unsigned i = 0;
            for (const auto& vertex : mesh.vertices()) {
                pts_raw[i*3+0]     = static_cast<float>(vertex->x());
                pts_raw[i*3+1]     = static_cast<float>(vertex->y());
                pts_raw[i*3+2]     = static_cast<float>(vertex->z());
                const Normal& n = mesh.normal(*vertex);
                normals_raw[i*3+0] = static_cast<float>(n.x());
                normals_raw[i*3+1] = static_cast<float>(n.y());
                normals_raw[i*3+2] = static_cast<float>(n.z());
                ++i;
            }
            unsigned vertex_number = mesh.vertices().size();
            os.write(reinterpret_cast<char*>(&vertex_number),sizeof(unsigned));
            os.write(reinterpret_cast<char*>(pts_raw),sizeof(float)*vertex_number*3);
            os.write(reinterpret_cast<char*>(&vertex_number),sizeof(unsigned));
            os.write(reinterpret_cast<char*>(normals_raw),sizeof(float)*vertex_number*3);

            delete[] normals_raw;
            delete[] pts_raw;

            //  Triangles

            unsigned* faces_raw = new unsigned[mesh.triangles().size()*3]; // Faces
            i = 0;
            for (const auto& triangle : mesh.triangles()) {
                faces_raw[i*3+0] = vertex_index(triangle,0);
                faces_raw[i*3+1] = vertex_index(triangle,1);
                faces_raw[i*3+2] = vertex_index(triangle,2);
                ++i;
            }

            unsigned zero = 0;
            os.write(reinterpret_cast<char*>(&zero),sizeof(unsigned));
            unsigned ntrgs = mesh.triangles().size();
            os.write(reinterpret_cast<char*>(&ntrgs),sizeof(unsigned));
            os.write(reinterpret_cast<char*>(faces_raw),sizeof(unsigned)*ntrgs*3);

            delete[] faces_raw;
        }

        MeshIO* clone(const std::string& filename) const override { return new Mesh(filename); }

    private:

        bool binary() const override { return true; }

        Mesh(const std::string& filename=""): base(filename,"mesh") { }

        static const Mesh prototype;

        const char* name() const override { return "MESH"; }
    };
}
