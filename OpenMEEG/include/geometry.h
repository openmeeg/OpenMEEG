// Project Name: OpenMEEG (http://openmeeg.github.io)
// © INRIA and ENPC under the French open source license CeCILL-B.
// See full copyright notice in the file LICENSE.txt
// If you make a copy of this file, you must either:
// - provide also LICENSE.txt and modify this header to refer to it.
// - replace this header by the LICENSE.txt content.

#pragma once

#include <iterator>
#include <string>
#include <vector>
#include <set>

#include <om_common.h>
#include <vertex.h>
#include <triangle.h>
#include <interface.h>
#include <domain.h>
#include <matrix.h>

#include <GeometryExceptions.H>

namespace OpenMEEG {

    class Mesh;

    /// \brief Geometry contains the electrophysiological model
    /// Vertices, meshes and domains are stored in this geometry.

    class OPENMEEG_EXPORT Geometry {
    public:

        struct MeshPair {

            MeshPair(const Mesh& m1,const Mesh& m2,const int o): meshes{&m1,&m2},orientation(o) { }

            const Mesh& operator()(const unsigned i) const { return *meshes[i]; }

            int relative_orientation() const { return orientation; }

        private:

            const Mesh* meshes[2];
            int         orientation;
        };

        typedef std::vector<MeshPair>                 MeshPairs;

        typedef std::vector<const Domain*>            DomainsReference;
        typedef std::vector<std::vector<const Mesh*>> MeshParts;

        typedef std::vector<std::pair<std::string,std::string>> MeshList;

        /// Constructors

        Geometry() {}

        Geometry(const std::string& geomFileName,const bool OLD_ORDERING=false) {
            load(geomFileName,OLD_ORDERING);
        }

        Geometry(const std::string& geomFileName,const std::string& condFileName,const bool OLD_ORDERING=false) {
            load(geomFileName,condFileName,OLD_ORDERING);
        }

        //  Absolutely necessary or wrong constructor is called because of conversion of char* to bool.

        Geometry(const char* geomFileName,const bool OLD_ORDERING=false): Geometry(std::string(geomFileName),OLD_ORDERING) { }
        Geometry(const char* geomFileName,const char* condFileName,const bool OLD_ORDERING=false):
            Geometry(std::string(geomFileName),std::string(condFileName),OLD_ORDERING) { }

        void info(const bool verbose=false) const; ///< \brief Print information on the geometry
        bool has_conductivities()           const { return conductivities; } // TODO: Is this useful ?
        bool selfCheck()                    const; ///< \brief the geometry meshes intersect each other
        bool check(const Mesh& m)           const; ///< \brief check if m intersect geometry meshes
        bool check_inner(const Matrix& m)   const; ///< \brief check if dipoles are outside of geometry meshes

        void check_geometry_is_nested();

        bool is_nested() const { return nested; }

        /// \brief Return the list of vertices involved in the geometry.

              Vertices& vertices()       { return geom_vertices; }
        const Vertices& vertices() const { return geom_vertices; }

        /// \brief Add a vertex \param V to the geometry and return the index of V in the vector of vertices.

        unsigned add_vertex(const Vertex& V) {
            // Insert the vertex in the set of vertices if it is not already in.

            const Vertices::iterator vit = std::find(vertices().begin(),vertices().end(),V);
            if (vit!=vertices().end())
                return vit-vertices().begin();

            vertices().push_back(V);
            return vertices().size()-1;
        }

        Mesh& add_mesh(const std::string& name="") {

            //  It is dangerous to store the returned mesh because the vector can be reallocated.
            //  Use mesh(name) after all meshes have been added....

            meshes().emplace_back(this);
            Mesh& mesh = meshes().back();
            mesh.name() = name;
            return mesh;
        }

        IndexMap add_vertices(const Vertices& vs) {
            IndexMap indmap;
            for (unsigned i=0; i<vs.size(); ++i)
                indmap.insert({ i, add_vertex(vs[i]) });
            return indmap;
        }

        /// \brief Return the list of meshes involved in the geometry.

              Meshes& meshes()       { return geom_meshes; }
        const Meshes& meshes() const { return geom_meshes; }

        const MeshPairs& communicating_mesh_pairs() const { return meshpairs; }

        /// \brief returns the Mesh called \param name .

        Mesh& mesh(const std::string& name);

        /// \brief  Return the list of domains.

              Domains& domains()       { return geom_domains; }
        const Domains& domains() const { return geom_domains; }

        /// \brief Get specific domains.

        const Domain& domain(const std::string& name) const; ///< \brief returns the Domain called \param name
        const Domain& domain(const Vect3& p)          const; ///< \brief returns the Domain containing the point p \param p a point

        /// \brief  Return the list of domains containing a mesh.

        DomainsReference domains(const Mesh& m) const {
            DomainsReference result;
            for (const auto& domain : domains())
                if (domain.contains(m))
                    result.push_back(&domain);
            return result;
        }

        size_t nb_parameters() const { return num_params; } ///< \brief the total number of vertices + triangles

        /// Returns the outermost domain.
        // It is unclear whether outermost_domain and set_outermost_domain need to be in the public interface.

        Domain& outermost_domain();

        void set_outermost_domain(Domain& domain) {
            outer_domain = &domain;
            for (auto& boundary : domain.boundaries())
                boundary.interface().set_to_outermost();
        }

        bool    is_outermost(const Domain& domain) const { return outer_domain==&domain; }

        const Interface& outermost_interface() const; ///< \brief returns the outermost interface (only valid for nested geometries).
        const Interface& innermost_interface() const; ///< \brief returns the innermost interface (only valid for nested geometries).

        const Interface& interface(const std::string& name) const; ///< \brief returns the Interface called \param name

        //  TODO: Find better names for the next two methods.

        double sigma    (const Mesh& m1,const Mesh& m2) const { return eval_on_common_domains<IDENTITY>(m1,m2);  } // return the (sum) conductivity(ies) of the shared domain(s).
        double sigma_inv(const Mesh& m1,const Mesh& m2) const { return eval_on_common_domains<INVERSE>(m1,m2);   } // return the (sum) inverse of conductivity(ies) of the shared domain(s).
        double indicator(const Mesh& m1,const Mesh& m2) const { return eval_on_common_domains<INDICATOR>(m1,m2); } // return the (sum) indicator function of the shared domain(s).

        /// \brief Return the conductivity jump across a mesh (i.e. between the 2 domains it separates).

        double conductivity_jump(const Mesh& m) const {
            const DomainsReference& doms = domains(m);
            double res = 0.0;
            for (const auto& domainptr : doms)
                res += domainptr->conductivity()*domainptr->mesh_orientation(m);
            return res;
        }

        /// \brief Give the relative orientation of two meshes:
        /// \return  0, if they don't have any domains in common
        ///          1, if they are both oriented toward the same domain
        ///         -1, if they are not

        int relative_orientation(const Mesh& m1,const Mesh& m2) const {
            if (&m1==&m2) // Fast path for identical meshes.
                return 1;
            const DomainsReference& doms = common_domains(m1,m2); // 2 meshes have either 0, 1 or 2 domains in common
            return (doms.size()==0) ? 0 : ((doms[0]->mesh_orientation(m1)==doms[0]->mesh_orientation(m2)) ? 1 : -1);
        }


        //  Calling this method read induces failures due do wrong conversions when read is passed with one or two arguments...

        void load(const std::string& filename,const bool OLD_ORDERING=false) {
            clear();
            read_geometry_file(filename);
            finalize(OLD_ORDERING);
        }

        void load(const std::string& geomFileName,const std::string& condFileName,const bool OLD_ORDERING=false) {
            clear();
            read_geometry_file(geomFileName);
            read_conductivity_file(condFileName);
            finalize(OLD_ORDERING);
        }

        void import(const MeshList& meshes);

        void save(const std::string& filename) const;

        void finalize(const bool OLD_ORDERING=false) {
            // TODO: We should check the correct decomposition of the geometry into domains here.
            // In a correct decomposition, each interface is used exactly once ?? Unsure...
            // Search for the outermost domain and set boolean OUTERMOST on the domain in the vector domains.
            // An outermost domain is defined as the only domain which has no inside. It is supposed to be
            // unique.

            if (has_conductivities())
                mark_current_barriers(); // mark meshes that touch the domains of null conductivity.

            if (domains().size()!=0) {
                set_outermost_domain(outermost_domain());
                check_geometry_is_nested();
            }

            generate_indices(OLD_ORDERING);
            make_mesh_pairs();
        }

        /// Handle multiple isolated domains

        size_t  nb_current_barrier_triangles() const { return nb_current_barrier_triangles_; }
        size_t& nb_current_barrier_triangles()       { return nb_current_barrier_triangles_; }
        size_t  nb_invalid_vertices()                { return invalid_vertices_.size();      }

        const MeshParts& isolated_parts() const { return independant_parts; }
              void       mark_current_barriers();
        const Mesh&      mesh(const std::string& id) const; //  Is this useful ?? TODO.

    private:

        void clear() {
            geom_vertices.clear();
            geom_meshes.clear();
            geom_domains.clear();
            conductivities = nested = false;
            outer_domain = 0;
            num_params = 0;
        }

        void read_geometry_file(const std::string& filename);
        void read_conductivity_file(const std::string& filename);

        void make_mesh_pairs();

        /// Members

        Vertices     geom_vertices;
        Meshes       geom_meshes;
        Domains      geom_domains;

        const Domain* outer_domain   = 0;
        bool          nested         = false;
        bool          conductivities = false; //    Is this really useful ??
        size_t        num_params     = 0;   // total number = nb of vertices + nb of triangles

        void  generate_indices(const bool);

        DomainsReference common_domains(const Mesh& m1,const Mesh& m2) const {
            const DomainsReference& doms1 = domains(m1);
            const DomainsReference& doms2 = domains(m2);
            DomainsReference doms;
            std::set_intersection(doms1.begin(),doms1.end(),doms2.begin(),doms2.end(),std::back_inserter(doms));
            return doms;
        }

        //  Accumulate a function over the domain common to two meshes.

        static double IDENTITY(const Domain& domain) { return domain.conductivity();     }
        static double INVERSE(const Domain& domain)  { return 1.0/domain.conductivity(); }
        static double INDICATOR(const Domain&)       { return 1.0;                       }

        template <double Function(const Domain&)>
        double eval_on_common_domains(const Mesh& m1,const Mesh& m2) const {
            const DomainsReference& doms = common_domains(m1,m2);
            double result = 0.0;
            for (const auto& domainptr : doms)
                result += Function(*domainptr);
            return result;
        }

        /// Handle multiple isolated domains.

        std::set<Vertex> invalid_vertices_;  ///< \brief  does not equal to the vertices of invalid meshes because there are shared vertices
        size_t           nb_current_barrier_triangles_ = 0;  ///< \brief number of triangles with 0 normal current. Including triangles of invalid meshes.

        MeshParts independant_parts;  ///< \brief Mesh names that belong to different isolated groups.
        MeshPairs meshpairs;
    };
}
