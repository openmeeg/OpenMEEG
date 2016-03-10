/* OpenMEEG

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

#include <fstream>
#include <cstring>
#include <sstream>
#include <stdexcept>

#include <mesh.h>
#include <integrator.h>
#include <cpuChrono.h>
#include <assemble.h>
#include <sensors.h>
#include <geometry.h>

using namespace std;
using namespace OpenMEEG;

unsigned gauss_order = 3;

void getHelp(char** argv);

int main(int argc, char** argv)
{
    print_version(argv[0]);

    bool OLD_ORDERING = false;
    if ( argc<2) {
        cerr << "Not enough arguments \nPlease try \"" << argv[0] << " -h\" or \"" << argv[0] << " --help \" \n" << endl;
        return 0;
    } else {
        OLD_ORDERING = (strcmp(argv[argc-1], "-old-ordering") == 0);
        if ( OLD_ORDERING ) {
            std::cout << "Using old ordering i.e using (V1, p1, V2, p2, V3) instead of (V1, V2, V3, p1, p2)" << std::endl;
        }
    }

    if ((!strcmp(argv[1],"-h")) | (!strcmp(argv[1],"--help"))) getHelp(argv);

    disp_argv(argc, argv);

    // Start Chrono
    cpuChrono C;
    C.start();

    /*********************************************************************************************
    * Computation of Head Matrix for BEM Symmetric formulation
    **********************************************************************************************/
    if ( ( !strcmp(argv[1], "-HeadMat") ) | ( !strcmp(argv[1], "-HM" ) ) | ( !strcmp(argv[1], "-hm") ) ) {
        if ( argc < 3 ) {
            std::cerr << "Please set geometry filepath !" << endl;
            exit(1);
        }
        if ( argc < 4 ) {
            std::cerr << "Please set conductivities filepath !" << endl;
            exit(1);
        }
        if ( argc < 5 ) {
            std::cerr << "Please set output filepath !" << endl;
            exit(1);
        }
        // Loading surfaces from geometry file
        Geometry geo;
        geo.read(argv[2], argv[3], OLD_ORDERING);

        // Check for intersecting meshes
        if ( !geo.selfCheck() ) {
            exit(1);
        }

        // Assembling Matrix from discretization :
        HeadMat HM(geo, gauss_order);
        HM.save(argv[4]);
    }

    /*********************************************************************************************
    * Computation of Cortical Matrix for BEM Symmetric formulation
    **********************************************************************************************/
    else if ( ( !strcmp(argv[1], "-CorticalMat") ) | ( !strcmp(argv[1], "-CM" ) ) | ( !strcmp(argv[1], "-cm") ) ) {
        if ( argc < 3 ) {
            std::cerr << "Please set geometry filepath !" << endl;
            exit(1);
        }
        if ( argc < 4 ) {
            std::cerr << "Please set conductivities filepath !" << endl;
            exit(1);
        }
        if ( argc < 5 ) {
            std::cerr << "Please set sensors !" << endl;
            exit(1);
        }
        if ( argc < 6 ) {
            std::cerr << "Please set the domain name !" << endl;
            exit(1);
        }
        if ( argc < 7 ) {
            std::cerr << "Please set output filepath !" << endl;
            exit(1);
        }
        double alpha, beta;
        if ( argc >= 9 ) {
            std::stringstream ss(argv[7]);
            if ( !(ss >> alpha) ) {
                throw std::runtime_error("given parameter is not a number");
            } else {
                ss.clear();
                ss.str(argv[8]);
                if ( !(ss >> beta) ) {
                    throw std::runtime_error("given parameter is not a number");
                }
            }
        }
        // Loading surfaces from geometry file
        Geometry geo;
        geo.read(argv[2], argv[3], OLD_ORDERING);

        // Check for intersecting meshes
        if ( !geo.selfCheck() ) {
            exit(1);
        }

        // read the file containing the positions of the EEG patches
        Sensors electrodes(argv[4]);
        Head2EEGMat M(geo, electrodes);

        // Assembling Matrix from discretization :
        CorticalMat *CM;
        if (argc == 9) {
            CM = new CorticalMat(geo, M, argv[5], gauss_order, alpha, beta);
        } else if (argc == 10) {
            CM = new CorticalMat(geo, M, argv[5], gauss_order, alpha, beta, argv[9]);
        } else {
            CM = new CorticalMat(geo, M, argv[5], gauss_order);
        }
        CM->save(argv[6]);
    }

    /*********************************************************************************************
    * Computation of general Surface Source Matrix for BEM Symmetric formulation
    **********************************************************************************************/
    else if ( ( !strcmp(argv[1], "-SurfSourceMat") ) | ( !strcmp(argv[1], "-SSM") ) | ( !strcmp(argv[1], "-ssm") ) ) {
        if ( argc < 3 ) {
            std::cerr << "Please set geometry filepath !" << endl;
            exit(1);
        }
        if ( argc < 4 ) {
            std::cerr << "Please set conductivities filepath !" << endl;
            exit(1);
        }
        if ( argc < 5 ) {
            std::cerr << "Please set 'mesh of sources' filepath !" << endl;
            exit(1);
        }

        // Loading surfaces from geometry file.
        Geometry geo;
        geo.read(argv[2], argv[3], OLD_ORDERING);

        // Loading mesh for distributed sources
        Mesh mesh_sources;
        mesh_sources.load(argv[4]);

        // Assembling Matrix from discretization :
        SurfSourceMat ssm(geo, mesh_sources, gauss_order);
        ssm.save(argv[5]); // if outfile is specified
    }

    /*********************************************************************************************
    * Computation of RHS for discrete dipolar case
    **********************************************************************************************/
    else if ( ( !strcmp(argv[1], "-DipSourceMat") ) | ( !strcmp(argv[1], "-DSM") ) | ( !strcmp(argv[1], "-dsm") ) | ( !strcmp(argv[1], "-DipSourceMatNoAdapt") ) | ( !strcmp(argv[1], "-DSMNA") ) | ( !strcmp(argv[1], "-dsmna") ) )  {
        if ( argc < 3 ) {
            cerr << "Please set geometry filepath !" << endl;
            exit(1);
        }
        if ( argc < 4 ) {
            std::cerr << "Please set conductivities filepath !" << endl;
            exit(1);
        }
        if ( argc < 5 ) {
            cerr << "Please set dipoles filepath!" << endl;
            exit(1);
        }
        std::string domain_name = "";
        if ( argc == 7 ) {
            domain_name = argv[6];
            std::cout << "Dipoles are considered to be in \"" << domain_name << "\" domain." << std::endl;
        }

        // Loading surfaces from geometry file.
        Geometry geo;
        geo.read(argv[2], argv[3], OLD_ORDERING);

        // Loading Matrix of dipoles :
        Matrix dipoles(argv[4]);
        if ( dipoles.ncol() != 6 ) {
            cerr << "Dipoles File Format Error" << endl;
            exit(1);
        }
        
        bool adapt_rhs = true;

        // Choosing between adaptive integration or not for the RHS
        if ( !strcmp(argv[1], "-DipSourceMatNoAdapt")|(!strcmp(argv[1], "-DSMNA"))|(!strcmp(argv[1], "-dsmna"))){
            adapt_rhs = false;
        }

        DipSourceMat dsm(geo, dipoles, gauss_order, adapt_rhs, domain_name);
        // Saving RHS Matrix for dipolar case :
        dsm.save(argv[5]);
    }

    /*********************************************************************************************
    * Computation of the RHS for EIT
    **********************************************************************************************/

    else if ( !strcmp(argv[1], "-EITSourceMat") | !strcmp(argv[1], "-EITSM") | !strcmp(argv[1], "-EITsm") ) {
        if ( argc < 3 ) {
            cerr << "Please set geometry filepath !" << endl;
            exit(1);
        }
        if ( argc < 4 ) {
            std::cerr << "Please set conductivities filepath !" << endl;
            exit(1);
        }
        if ( argc < 5 ) {
            std::cerr << "Please set electrode positions filepath !" << endl;
            exit(1);
        }
        if ( argc < 6 ) {
            std::cerr << "Please set output EITSourceMat filepath !" << endl;
            exit(1);
        }

        // Loading surfaces from geometry file.
        Geometry geo;
        geo.read(argv[2], argv[3], OLD_ORDERING);

        Sensors electrodes(argv[4], geo); // special parameter for EIT electrodes: the interface
        EITSourceMat EITsource(geo, electrodes, gauss_order);
        EITsource.save(argv[5]);
    }

    /*********************************************************************************************
    * Computation of the linear application which maps the unknown vector in symmetric system,
    * (i.e. the potential and the normal current on all interfaces)
    * |----> v (potential at the electrodes)
    **********************************************************************************************/
    else if ( !strcmp(argv[1], "-Head2EEGMat") | !strcmp(argv[1], "-H2EM") | !strcmp(argv[1], "-h2em") ) {

        if ( argc < 3 ) {
            cerr << "Please set geometry filepath !" << endl;
            exit(1);
        }
        if ( argc < 4 ) {
            std::cerr << "Please set conductivities filepath !" << endl;
            exit(1);
        }
        if ( argc < 5 ) {
            cerr << "Please set electrode positions filepath !" << endl;
            exit(1);
        }

        // Loading surfaces from geometry file.
        Geometry geo;
        geo.read(argv[2], argv[3], OLD_ORDERING);

        // read the file containing the positions of the EEG patches
        Sensors electrodes(argv[4]);

        // Assembling Matrix from discretization :
        // Head2EEG is the linear application which maps x |----> v
        Head2EEGMat mat(geo, electrodes);
        // Saving Head2EEG Matrix :
        mat.save(argv[5]);
    }

    /*********************************************************************************************
    * Computation of the linear application which maps the unknown vector in symmetric system,
    * (i.e. the potential and the normal current on all interfaces)
    * |----> v (potential at the ECoG electrodes)
    **********************************************************************************************/
    else if ( !strcmp(argv[1], "-Head2ECoGMat") | !strcmp(argv[1], "-H2ECOGM") | !strcmp(argv[1], "-H2ECoGM") | !strcmp(argv[1], "-h2ecogm")) {

        if ( argc < 3 ) {
            cerr << "Please set geometry filepath !" << endl;
            exit(1);
        }
        if ( argc < 4 ) {
            std::cerr << "Please set conductivities filepath !" << endl;
            exit(1);
        }
        if ( argc < 5 ) {
            cerr << "Please set ECoG electrode positions filepath !" << endl;
            exit(1);
        }
        if ( argc < 6 ) {
            cerr << "Please set the name of the interface for EcoG" << endl;
            exit(1);
        }
        if ( argc < 7 ) {
            cerr << "Please set output matrix filepath !" << endl;
            exit(1);
        }

        // Loading surfaces from geometry file.
        Geometry geo;
        geo.read(argv[2], argv[3], OLD_ORDERING);

        // Find the mesh of the Ecog electrodes
        const Interface &i = geo.interface(argv[5]);

        // read the file containing the positions of the EEG patches
        Sensors electrodes(argv[4]);

        // Assembling Matrix from discretization :
        // Head2ECoG is the linear application which maps x |----> v
        Head2ECoGMat mat(geo, electrodes, i);

        // Saving Head2ECoG Matrix :
        mat.save(argv[6]);
    }

    /*********************************************************************************************
    * Computation of the linear application which maps the unknown vector in symmetric system,
    * (i.e. the potential and the normal current on all interfaces)
    * |----> bFerguson (contrib to MEG response)
    **********************************************************************************************/
    else if ( !strcmp(argv[1], "-Head2MEGMat") | !strcmp(argv[1], "-H2MM") | !strcmp(argv[1], "-h2mm") ) {

        if ( argc < 3) {
            cerr << "Please set geometry filepath !" << endl;
            exit(1);
        }
        if ( argc < 4) {
            std::cerr << "Please set conductivities filepath !" << endl;
            exit(1);
        }
        if ( argc < 5) {
            cerr << "Please set squids filepath !" << endl;
            exit(1);
        }

        // Loading surfaces from geometry file.
        Geometry geo;
        geo.read(argv[2], argv[3]);

        // Load positions and orientations of sensors  :
        Sensors sensors(argv[4]);

        // Assembling Matrix from discretization :
        Head2MEGMat mat(geo, sensors);
        // Saving Head2MEG Matrix :
        mat.save(argv[5]); // if outfile is specified
    }

    /*********************************************************************************************
    * Computation of the linear application which maps the distributed source
    * |----> binf (contrib to MEG response)
    **********************************************************************************************/
    else if ( !strcmp(argv[1], "-SurfSource2MEGMat") | !strcmp(argv[1], "-SS2MM") | !strcmp(argv[1], "-ss2mm") ) {

        if ( argc < 3 ) {
            cerr << "Please set 'mesh sources' filepath !" << endl;
            exit(1);
        }
        if ( argc < 4 ) {
            cerr << "Please set squids filepath !" << endl;
            exit(1);
        }

        // Loading mesh for distributed sources :
        Geometry geo;
        Mesh mesh_sources;
        mesh_sources.load(argv[2]);
        // Load positions and orientations of sensors  :
        Sensors sensors(argv[3]);

        // Assembling Matrix from discretization :
        SurfSource2MEGMat mat(mesh_sources, sensors);
        // Saving SurfSource2MEG Matrix :
        mat.save(argv[4]);
    }

    /*********************************************************************************************
    * Computation of the discrete linear application which maps s (the dipolar source)
    * |----> binf (contrib to MEG response)
    **********************************************************************************************/
    // arguments are the positions and orientations of the squids,
    // the position and orientations of the sources and the output name.

    else if ( !strcmp(argv[1], "-DipSource2MEGMat") | !strcmp(argv[1], "-DS2MM") | !strcmp(argv[1], "-ds2mm")) {

        if ( argc < 3 ) {
            cerr << "Please set dipoles filepath !" << endl;
            exit(1);
        }
        if ( argc < 4 ) {
            cerr << "Please set squids filepath !" << endl;
            exit(1);
        }

        // Loading dipoles :
        Matrix dipoles(argv[2]);

        // Load positions and orientations of sensors  :
        Sensors sensors(argv[3]);

        DipSource2MEGMat mat( dipoles, sensors );
        mat.save(argv[4]);
    }

    /*********************************************************************************************
    * Computation of the discrete linear application which maps x (the unknown vector in a symmetric system)
    * |----> v, potential at a set of prescribed points within the 3D volume
    **********************************************************************************************/

    else if ( !strcmp(argv[1], "-Head2InternalPotMat") | !strcmp(argv[1], "-H2IPM") | !strcmp(argv[1], "-h2ipm") ) {
        if ( argc < 3 ) {
            cerr << "Please set geom filepath !" << endl;
            exit(1);
        }
        if ( argc < 4 ) {
            cerr << "Please set cond filepath !" << endl;
            exit(1);
        }
        if ( argc < 5 ) {
            cerr << "Please set point positions filepath !" << endl;
            exit(1);
        }
        if ( argc < 6 ) {
            std::cerr << "Please set output filepath !" << endl;
            exit(1);
        }
        // Loading surfaces from geometry file
        Geometry geo;
        geo.read(argv[2], argv[3], OLD_ORDERING);
        Matrix points(argv[4]);
        Surf2VolMat mat(geo, points);
        // Saving SurfToVol Matrix :
        mat.save(argv[5]);
    }
    /*********************************************************************************************
    * Computation of the discrete linear application which maps the dipoles
    * |----> Vinf, potential at a set of prescribed points within the volume, in an infinite medium
    *    Vinf(r)=1/(4*pi*sigma)*(r-r0).q/(||r-r0||^3)
    **********************************************************************************************/

    else if ( !strcmp(argv[1], "-DipSource2InternalPotMat") | !strcmp(argv[1], "-DS2IPM") | !strcmp(argv[1], "-ds2ipm") ) {
        if (argc<3) {
            cerr << "Please set geom filepath !" << endl;
            exit(1);
        }
        if (argc<4) {
            cerr << "Please set cond filepath !" << endl;
            exit(1);
        }
        if (argc<5) {
            cerr << "Please set dipoles filepath !" << endl;
            exit(1);
        }
        if (argc<6) {
            cerr << "Please set point positions filepath !" << endl;
            exit(1);
        }
        if (argc<7) {
            std::cerr << "Please set output filepath !" << endl;
            exit(1);
        }
        std::string domain_name = "";
        if (argc==9) {
            domain_name = argv[7];
            std::cout << "Dipoles are considered to be in \"" << domain_name << "\" domain." << std::endl;
        }
        // Loading surfaces from geometry file
        Geometry geo;
        geo.read(argv[2],argv[3],OLD_ORDERING);
        // Loading dipoles :
        Matrix dipoles(argv[4]);
        Matrix points(argv[5]);
        DipSource2InternalPotMat mat(geo, dipoles, points, domain_name);
        mat.save(argv[6]);
    }

    else cerr << "unknown argument: " << argv[1] << endl;

    // Stop Chrono
    C.stop();
    C.dispEllapsed();
}

void getHelp(char** argv) {
    cout << argv[0] <<" [-option] [filepaths...]" << endl << endl;

    cout << "option :" << endl;
    cout << "   -HeadMat, -HM, -hm :   " << endl;
    cout << "       Compute Head Matrix for Symmetric BEM (left-hand side of linear system)." << endl;
    cout << "             Arguments :" << endl;
    cout << "               geometry file (.geom)" << endl;
    cout << "               conductivity file (.cond)" << endl;
    cout << "               output matrix" << endl << endl;

    cout << "   -CorticalMat, -CM, -cm :   " << endl;
    cout << "       Compute Cortical Matrix for Symmetric BEM (left-hand side of linear system)." << endl;
    cout << "             Arguments :" << endl;
    cout << "               geometry file (.geom)" << endl;
    cout << "               conductivity file (.cond)" << endl;
    cout << "               file containing the positions of EEG electrodes (.patches)" << endl;
    cout << "               domain name (containing the sources)" << endl;
    cout << "               output matrix" << endl << endl;

    cout << "   -SurfSourceMat, -SSM, -ssm :   " << endl;
    cout << "       Compute Surfacic Source Matrix for Symmetric BEM (right-hand side of linear system). " << endl;
    cout << "            Arguments :" << endl;
    cout << "               geometry file (.geom)" << endl;
    cout << "               conductivity file (.cond)" << endl;
    cout << "               mesh of sources (.tri .vtk .mesh .bnd)" << endl;
    cout << "               output matrix" << endl << endl;

    cout << "   -DipSourceMat, -DSM, -dsm:    " << endl;
    cout << "      Compute Dipolar Source Matrix for Symmetric BEM (right-hand side of linear system). " << endl;
    cout << "            Arguments :" << endl;
    cout << "               geometry file (.geom)" << endl;
    cout << "               conductivity file (.cond)" << endl;
    cout << "               dipoles positions and orientations" << endl;
    cout << "               output matrix" << endl;
    cout << "               (Optional) domain name where lie all dipoles." << endl << endl;

    cout << "   -EITSourceMat, -EITSM -EITsm : " << endl;
    cout << "       Compute the EIT Source Matrix from an injected current (right-hand side of linear system). " << endl;
    cout << "            Arguments :" << endl;
    cout << "               geometry file (.geom)" << endl;
    cout << "               conductivity file (.cond)" << endl;
    cout << "               file containing the positions of EIT electrodes (.patches)" << endl;
    cout << "               output EITSourceOp" << endl;

    cout << "   -Head2EEGMat, -H2EM, -h2em : " << endl;
    cout << "        Compute the linear application which maps the potential" << endl;
    cout << "        on the scalp to the EEG electrodes"  << endl;
    cout << "            Arguments :" << endl;
    cout << "               geometry file (.geom)" << endl;
    cout << "               conductivity file (.cond)" << endl;
    cout << "               file containing the positions of EEG electrodes (.patches)" << endl;
    cout << "               output matrix" << endl << endl;

    cout << "   -Head2ECoGMat, -H2ECogM, -h2ecogm, -H2ECOGM : " << endl;
    cout << "        Compute the linear application which maps the potential" << endl;
    cout << "        on the scalp to the ECoG electrodes"  << endl;
    cout << "            Arguments :" << endl;
    cout << "               geometry file (.geom)" << endl;
    cout << "               conductivity file (.cond)" << endl;
    cout << "               file containing the positions of ECoG electrodes (.patches)" << endl;
    cout << "               name of the interface on which to project the electrodes (\"name\")" << endl;
    cout << "               output matrix" << endl << endl;

    cout << "   -Head2MEGMat, -H2MM, -h2mm : " << endl;
    cout << "        Compute the linear application which maps the potential" << endl;
    cout << "        on the scalp to the MEG sensors"  << endl;
    cout << "            Arguments :" << endl;
    cout << "               geometry file (.geom)" << endl;
    cout << "               conductivity file (.cond)" << endl;
    cout << "               file containing the positions and orientations of the MEG sensors (.squids)" << endl;
    cout << "               output matrix" << endl << endl;

    cout << "   -SurfSource2MEGMat, -SS2MM, -ss2mm : " << endl;
    cout << "        Compute the linear application which maps the " << endl;
    cout << "        distributed source  to the MEG sensors" << endl;
    cout << "            Arguments :" << endl;
    cout << "               mesh file for distributed sources (.tri .vtk .mesh .bnd)" << endl;
    cout << "               positions and orientations of the MEG sensors (.squids)" << endl;
    cout << "               output matrix" << endl << endl;

    cout << "   -DipSource2MEGMat, -DS2MM, -ds2mm :  " << endl;
    cout << "        Compute the linear application which maps the current dipoles" << endl;
    cout << "        to the MEG sensors" << endl;
    cout << "            Arguments :" << endl;
    cout << "               dipoles positions and orientations" << endl;
    cout << "               positions and orientations of the MEG sensors (.squids)" << endl;
    cout << "               output matrix" << endl << endl;

    cout << "   -Head2InternalPotMat, -H2IPM -h2ipm :  " << endl;
    cout << "        Compute the linear transformation which maps the surface potential" << endl;
    cout << "        and normal current to the value of the internal potential at a set of points within a volume" << endl;
    cout << "            Arguments :" << endl;
    cout << "               geometry file (.geom)" << endl;
    cout << "               conductivity file (.cond)" << endl;
    cout << "               a mesh file or a file with point positions at which to evaluate the potential" << endl;
    cout << "               output matrix" << endl << endl;

    cout << "   -DipSource2InternalPotMat, -DS2IPM -ds2ipm :   " << endl;
    cout << "        Compute the linear transformation  which maps the current dipoles" << endl;
    cout << "        to the value of the infinite potential at a set of points within a volume" << endl;
    cout << "            Arguments :" << endl;
    cout << "               geometry file (.geom)" << endl;
    cout << "               conductivity file (.cond)" << endl;
    cout << "               dipoles positions and orientations" << endl;
    cout << "               a mesh file or a file with point positions at which to evaluate the potential" << endl;
    cout << "               output matrix" << endl;
    cout << "               (Optional) domain name where lie all dipoles." << endl << endl;

    exit(0);
}
