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

#if WIN32
#define _USE_MATH_DEFINES
#endif
#include <math.h>

#include <vector.h>
#include <matrix.h>
#include <danielsson.h>
#include <operators.h>
#include <assemble.h>
#include <sensors.h>
#include <fstream>

namespace OpenMEEG {

    void assemble_SurfSourceMat(Matrix& mat, const Geometry& geo, Mesh& mesh_source, const unsigned gauss_order) 
    {
        mat = Matrix((geo.size()-geo.outermost_interface().nb_triangles()), mesh_source.nb_vertices());
        mat.set(0.0);

        // check if no overlapping between the geometry and the source mesh
        bool OK = geo.check(mesh_source);
        if ( !OK ) {
            std::cerr << "Error: source mesh overlapps the geometry" << std::endl;
            return;
        } // then the mesh is included in a domain of the geometry

        const Domain d     = geo.domain(**mesh_source.vertex_begin()); 
        const double sigma = d.sigma();
        const double K     = 1.0/(4.*M_PI);

        const unsigned nVertexSources = mesh_source.nb_vertices();
        
        // We here set it as an outermost (to tell _operarorN it doesn't belong to the geometry)
        mesh_source.outermost() = true;

        std::cout << std::endl << "assemble SurfSourceMat with " << nVertexSources << " mesh_source located in domain \"" << d.name() << "\"." << std::endl << std::endl;

        for ( Domain::const_iterator hit = d.begin(); hit != d.end(); ++hit) {
            for ( Interface::const_iterator omit = hit->interface().begin(); omit != hit->interface().end(); ++omit) {
                // First block is nVertexFistLayer*nVertexSources.
                double coeffN = (hit->inside())?K * omit->orientation() : omit->orientation() * -K;
                operatorN( omit->mesh(), mesh_source, mat, coeffN, gauss_order);
                // Second block is nFacesFistLayer*nVertexSources.
                double coeffD = (hit->inside())?-omit->orientation() * K / sigma : omit->orientation() * K / sigma;
                operatorD(omit->mesh(), mesh_source, mat, coeffD, gauss_order,false);
            }
        }
    }

    SurfSourceMat::SurfSourceMat(const Geometry& geo, Mesh& mesh_source, const unsigned gauss_order) 
    {
        assemble_SurfSourceMat(*this, geo, mesh_source, gauss_order);
    }

    void assemble_DipSourceMat(Matrix& rhs, const Geometry& geo, const Matrix& dipoles,
            const unsigned gauss_order, const bool adapt_rhs, const std::string& domain_name = "") 
    {
        const double   K         = 1.0/(4.*M_PI);
        const unsigned size      = (geo.size() - geo.outermost_interface().nb_triangles());
        const unsigned n_dipoles = dipoles.nlin();

        rhs = Matrix(size, n_dipoles);
        rhs.set(0.);

        Vector rhs_col(rhs.nlin());
        for ( unsigned s = 0; s < n_dipoles; ++s) {
            PROGRESSBAR(s, n_dipoles);
            const Vect3 r(dipoles(s, 0), dipoles(s, 1), dipoles(s, 2));
            const Vect3 q(dipoles(s, 3), dipoles(s, 4), dipoles(s, 5));

            Domain domain;

            if ( domain_name == "" ) {
                domain = geo.domain(r);
            } else {
                domain = geo.domain(domain_name);
            }
            const double sigma = domain.sigma();

            rhs_col.set(0.);
            // iterate over the domain's interfaces (half-spaces)
            for ( Domain::const_iterator hit = domain.begin(); hit != domain.end(); ++hit ) {
                // iterate over the meshes of the interface
                for ( Interface::const_iterator omit = hit->interface().begin(); omit != hit->interface().end(); ++omit ) {
                    //  Treat the mesh.
                    double coeffD = (hit->inside())?(K * omit->orientation()):(-K * omit->orientation());
                    operatorDipolePotDer(r, q, omit->mesh(), rhs_col, coeffD, gauss_order, adapt_rhs);

                    if ( !omit->mesh().outermost() ) {
                        double coeff = ( hit->inside() )?(-omit->orientation() * K / sigma):(omit->orientation() * K / sigma);
                        operatorDipolePot(r, q, omit->mesh(), rhs_col, coeff, gauss_order, adapt_rhs);
                    }
                }
            }
            rhs.setcol(s, rhs_col);
        }
    }

    DipSourceMat::DipSourceMat(const Geometry& geo, const Matrix& dipoles, const unsigned gauss_order,
                               const bool adapt_rhs, const std::string& domain_name)
    {
        assemble_DipSourceMat(*this, geo, dipoles, gauss_order, adapt_rhs, domain_name);
    }

    void assemble_EITSourceMat(Matrix& mat, const Geometry& geo, const Sensors& electrodes, const unsigned gauss_order)
    {
        //  A Matrix to be applied to the scalp-injected current to obtain the Source Term of the EIT foward problem.

        unsigned n_sensors = electrodes.getNumberOfSensors();

        const double K = 1.0/(4.*M_PI);

        //  transmat = a big SymMatrix of which mat = part of its transpose.
        SymMatrix transmat(geo.size());
        transmat.set(0.0);
        mat = Matrix((geo.size()-geo.outermost_interface().nb_triangles()), n_sensors);
        mat.set(0.);

        const Interface& i = geo.outermost_interface();

        // We iterate over the meshes (or pair of domains)
        for ( Interface::const_iterator omit1 = i.begin(); omit1 != i.end(); ++omit1) {
            for ( Geometry::const_iterator mit2 = geo.begin(); mit2 != geo.end(); ++mit2) {

                const int orientation = geo.oriented(omit1->mesh(), *mit2); // equals  0, if they don't have any domains in common
                                                                  // equals  1, if they are both oriented toward the same domain
                                                                  // equals -1, if they are not
                if ( orientation != 0 ) {
                    //  Compute S.
                    operatorS(*mit2, omit1->mesh(), transmat, geo.sigma_inv(omit1->mesh(), *mit2) * ( -1. * K * orientation), gauss_order);

                    //  First compute D.
                    operatorD(*mit2, omit1->mesh(), transmat, (K * orientation), gauss_order, true);
                    if ( omit1->mesh() == *mit2 ) {
                        operatorP1P0(omit1->mesh(), transmat, 0.5 * orientation);
                    }
                }
            }
        }

        for ( unsigned ielec = 0; ielec < n_sensors; ++ielec) {
            Triangles tris = electrodes.getInjectionTriangles(ielec);
            for ( Triangles::const_iterator tit = tris.begin(); tit != tris.end(); ++tit) {
                // to ensure exactly no accumulation of currents. w = elec_area/tris_area (~= 1)
                double inv_area = electrodes.getWeights()(ielec);
                // if no radius is given, we assume the user wants to specify an intensity not a density of current
                if ( electrodes.getRadius()(0) < 1e3*std::numeric_limits<double>::epsilon() ) {
                    inv_area = 1./tit->area();
                }
                for ( unsigned i = 0; i < (geo.size() - geo.outermost_interface().nb_triangles()); ++i) {
                    mat(i, ielec) += transmat(tit->index(), i) * inv_area;
                }
            }
        }
    }

    EITSourceMat::EITSourceMat(const Geometry& geo, const Sensors& electrodes, const unsigned gauss_order) 
    {
        assemble_EITSourceMat(*this, geo, electrodes, gauss_order);
    }

    void assemble_DipSource2InternalPotMat(Matrix& mat, const Geometry& geo, const Matrix& dipoles,
                                           const Matrix& points, const std::string& domain_name)     
    {
        // Points with one more column for the index of the domain they belong
        std::vector<Domain> points_domain;
        std::vector<Vect3>  points_;
        for ( unsigned i = 0; i < points.nlin(); ++i) {
            const Domain& d = geo.domain(Vect3(points(i, 0), points(i, 1), points(i, 2)));
            if ( d.name() != "Air" ) {
                points_domain.push_back(d);
                points_.push_back(Vect3(points(i, 0), points(i, 1), points(i, 2)));
            }
            else {
                std::cerr << " DipSource2InternalPot: Point [ " << points.getlin(i);
                std::cerr << "] is outside the head. Point is dropped." << std::endl;
            }
        }
        const double K = 1.0/(4.*M_PI);
        mat = Matrix(points_.size(), dipoles.nlin());
        mat.set(0.0);

        for ( unsigned iDIP = 0; iDIP < dipoles.nlin(); ++iDIP) {
            const Vect3 r0(dipoles(iDIP, 0), dipoles(iDIP, 1), dipoles(iDIP, 2));
            const Vect3  q(dipoles(iDIP, 3), dipoles(iDIP, 4), dipoles(iDIP, 5));

            Domain domain;
            if ( domain_name == "" ) {
                domain = geo.domain(r0);
            } else {
                domain = geo.domain(domain_name);
            }
            const double sigma  = domain.sigma();

            static analyticDipPot anaDP;
            anaDP.init(q, r0);
            for ( unsigned iPTS = 0; iPTS < points_.size(); ++iPTS) {
                if ( points_domain[iPTS] == domain ) {
                    mat(iPTS, iDIP) += K/sigma*anaDP.f(points_[iPTS]);
                }
            }
        }
    }

    DipSource2InternalPotMat::DipSource2InternalPotMat(const Geometry& geo, const Matrix& dipoles,
                                                       const Matrix& points, const std::string& domain_name)
    {
        assemble_DipSource2InternalPotMat(*this, geo, dipoles, points, domain_name);
    }
}
