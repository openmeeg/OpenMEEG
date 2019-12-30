%module(docstring="OpenMEEG bindings for python") openmeeg

%include <exception.i>
%exception {
    try {
        $action
    } catch (const std::exception& e) {
        SWIG_exception(SWIG_RuntimeError, e.what());
    }
}

#ifdef SWIGWIN
%include <windows.i>
#endif

#ifdef DOCSTRINGS
%include <docstrings.i>
#endif

%include <std_string.i>
%include <std_vector.i>

%{
    #define SWIG_FILE_WITH_INIT
    #include <vect3.h>
    #include <vertex.h>
    #include <triangle.h>
    #include <linop.h>
    #include <vector.h>
    #include <matrix.h>
    #include <symmatrix.h>
    #include <sparse_matrix.h>
    #include <fast_sparse_matrix.h>
    #include <sensors.h>
    #include <geometry.h>
    #include <GeometryIO.h>
    #include <mesh.h>
    #include <interface.h>
    #include <domain.h>
    #include <assemble.h>
    #include <gain.h>
    #include <forward.h>
    #include <iostream>

    #ifdef SWIGPYTHON

        #define NPY_NO_DEPRECATED_API NPY_1_7_API_VERSION
        #include <numpy/arrayobject.h>

    #endif

    using namespace OpenMEEG;
%}

// /////////////////////////////////////////////////////////////////
// Preprocessing setup
// /////////////////////////////////////////////////////////////////

#pragma SWIG nowarn=302, 315, 389, 401, 509, 801, 472, 473, 476, 362, 503, 514, 516, 842, 845

// /////////////////////////////////////////////////////////////////
// Ignore rules for operators
// /////////////////////////////////////////////////////////////////

%ignore operator>>;
%ignore operator<<;
%ignore operator==;
%ignore operator[];
%ignore operator!=;
%ignore operator*=;
%ignore operator/=;
//%ignore operator bool;
//%ignore operator int;
//%ignore operator float;
//%ignore operator double;
//%ignore operator double *;

// /////////////////////////////////////////////////////////////////
// Legacy
// /////////////////////////////////////////////////////////////////

%include "numpy.i"

%init %{
import_array();
%}

%exception {
    try {
        $action
    }
    catch (std::exception& e) {
        PyErr_SetString(PyExc_RuntimeError,e.what());
        return NULL;
    }
}

// /////////////////////////////////////////////////////////////////
// Definitions
// /////////////////////////////////////////////////////////////////

#define OPENMEEGMATHS_EXPORT
#define OPENMEEG_EXPORT

#ifdef 0  // POC for translating tuple(Vertex *) to numpy.array
#ifdef SWIGPYTHON

namespace OpenMEEG {

    // C++ -> Python

    %typemap(out) Vertex& {
        npy_intp shape[1];
        shape[0] = 3;

        double &data = ($1)->x();

        $result = PyArray_SimpleNewFromData(1,shape,NPY_DOUBLE,static_cast<void*>(&data));
    }

    %typemap(out) PVertices & {
        std::cerr << "Calling TYPEMAP OUT PVertices & (NOT IMPLEMENTED)" << std::endl;
    }

    %typemap(out) Mesh::VectPVertex &  {
        std::cerr << "Calling TYPEMAP OUT Mesh::VectPVertex & " << std::endl;

        npy_intp shape[2];
        shape[0] = ($1)->size();
        shape[1] = 4;

        double &data = ($1)->at(0)->x();

        $result = PyArray_SimpleNewFromData(2, shape, NPY_DOUBLE, static_cast<void*>(&data));
    }
}

#endif // SWIGPYTHON
#endif // 0

namespace std {
    %template(vector_int) vector<int>;
    %template(vector_unsigned) vector<unsigned int>;
    %template(vector_double) vector<double>;
    %template(vector_vertex) vector<OpenMEEG::Vertex>;
    %template(vector_pvertex) vector<OpenMEEG::Vertex *>;
    %template(vector_triangle) vector<OpenMEEG::Triangle>;
    %template(vector_string) vector<std::string>;
    %template(vector_interface) vector<OpenMEEG::Interface>;
    %template(vector_simple_dom) vector<OpenMEEG::SimpleDomain>;
    %template(vector_domain) vector<OpenMEEG::Domain>;
    %template(vector_oriented_mesh) vector<OpenMEEG::OrientedMesh>;
}

namespace OpenMEEG {
    // The OpenMEEG:: prefix seems required...
    // Otherwise some wrapping tests are failing.
    %typedef std::vector<OpenMEEG::Vertex>   Vertices;
    %typedef std::vector<OpenMEEG::Vertex*>  PVertices;
    %typedef std::vector<OpenMEEG::Triangle> Triangles;
    %typedef std::vector<OpenMEEG::Domain>   Domains;
    %typedef std::vector<std::string>        Strings;
    %typedef std::vector<SimpleDomain>       Boundaries;
    %typedef std::vector<OrientedMesh>       OrientedMeshes;
}

namespace OpenMEEG {

    %naturalvar Mesh;
    class Mesh;

    %naturalvar Matrix;
    class Matrix;

    %naturalvar Triangle;
    class Triangle;

    %naturalvar Vect3;
    class Vect3;

    %naturalvar Vertex;
    class Vertex;

    %naturalvar Vector;
    class Vector;

    %naturalvar Domains;
    class Domains;

    %naturalvar Boundaries;
    class Boundaries;

    %naturalvar OrientedMeshes;
    class OrientedMeshes;
}

#ifdef SWIGPYTHON

%inline %{

    // Creator of Vector from PyArrayObject or Vector

    OpenMEEG::Vector* new_OpenMEEG_Vector(PyObject* o) {
        if (PyArray_Check(o)) {
            PyArrayObject *vect = (PyArrayObject *) PyArray_FromObject(o, NPY_DOUBLE, 1, 1);
            const size_t nelem = PyArray_DIM(vect, 0);
            OpenMEEG::Vector *v = new Vector(nelem);
            v->reference_data(static_cast<double *>(PyArray_GETPTR1(vect, 0)));
            return v;
        }

        //  If the object is an OpenMEEG vector converted to python, copy the vector.
        //  TODO: do we need this ???

        void *ptr = 0 ;
        if (!SWIG_IsOK(SWIG_ConvertPtr(o,&ptr,SWIGTYPE_p_OpenMEEG__Vector,SWIG_POINTER_EXCEPTION))) {
            PyErr_SetString(PyExc_TypeError, "Input object is neither a PyArray nor a Vector.");
            return nullptr;
        }

        return new Vector(*(reinterpret_cast<OpenMEEG::Vector *>(ptr)), DEEP_COPY);
    }

    // Creator of Matrix from PyArrayObject or Matrix

    OpenMEEG::Matrix* new_OpenMEEG_Matrix(PyObject* o) {
        if (PyArray_Check(o)) {
            const int nbdims = PyArray_NDIM(reinterpret_cast<PyArrayObject*>(o));
            if (nbdims!=2) {
                PyErr_SetString(PyExc_TypeError, "Matrix can only have 2 dimensions.");
                return nullptr;
            }

            PyArrayObject* mat = reinterpret_cast<PyArrayObject*>(PyArray_FromObject(o,NPY_DOUBLE,2,2));

            if (!PyArray_ISFARRAY(mat)) {
                PyErr_SetString(PyExc_TypeError, "Matrix requires the use of Fortran order.");
                return nullptr;
            }

            const size_t nblines = PyArray_DIM(mat,0);
            const size_t nbcol   = PyArray_DIM(mat,1);

            OpenMEEG::Matrix* result = new Matrix(nblines,nbcol);
            result->reference_data(static_cast<double*>(PyArray_GETPTR2(mat,0,0)));
            return result;
        }

        //  If the object is an OpenMEEG matrix converted to python, just return the matrix.

        void* ptr = 0;
        if (!SWIG_IsOK(SWIG_ConvertPtr(o,&ptr,SWIGTYPE_p_OpenMEEG__Matrix,SWIG_POINTER_EXCEPTION))) {
            PyErr_SetString(PyExc_TypeError, "Input object must be a PyArray or an OpenMEEG Matrix.");
            return nullptr;
        }

        return new Matrix(*(reinterpret_cast<OpenMEEG::Matrix*>(ptr)));
    }
%}

// /////////////////////////////////////////////////////////////////
// Typemaps
// /////////////////////////////////////////////////////////////////

namespace OpenMEEG {

    // Python -> C++
    %typemap(in) Vector& {
        $1 = new_OpenMEEG_Vector($input);
    }

    %typemap(freearg) Vector& {
        if ($1) delete $1;
    }

    %typemap(in) Matrix& {
        $1 = new_OpenMEEG_Matrix($input);
    }

    %typemap(freearg) Matrix& {
        if ($1) delete $1;
    }

    // C++ -> Python

    %typemap(out) unsigned& {
        $result = PyInt_FromLong(*($1));
    }
}
#endif // SWIGPYTHON

// /////////////////////////////////////////////////////////////////
// extensions
// /////////////////////////////////////////////////////////////////

// OpenMEEG

%ignore OpenMEEG::Filetype;

// OpenMEEG/Vertex

%extend OpenMEEG::Vertex {

    double x() {
        return (($self)->x());
    }
    double y() {
        return (($self)->y());
    }
    double z() {
        return (($self)->z());
    }

    PyObject* array() {
        npy_intp shape[1];
        shape[0] = 3;

        double& data = ($self)->x();
        return PyArray_SimpleNewFromData(1, shape, NPY_DOUBLE, static_cast<void*>(&data));
    }
}

// OpenMEEG::Triangle

%extend OpenMEEG::Triangle {
    double area() {
        return (($self)->area());
    }
}

// OpenMEEG::Vector

%extend OpenMEEG::Vector {
    Vector(PyObject* o) {
        return new_OpenMEEG_Vector(o);
    }

    PyObject* array() {
        const npy_intp ndims = 1;
        npy_intp ar_dim[] = { static_cast<npy_intp>(($self)->size()) };
        PyArrayObject* array = reinterpret_cast<PyArrayObject*>(PyArray_SimpleNewFromData(ndims,ar_dim,NPY_DOUBLE,static_cast<void*>(($self)->data())));
        return PyArray_Return(array);
    }

    // Setters

    void setvalue(const unsigned int i,const double d) {
        (*($self))(i) = d;
    }

    double value(unsigned int i) {
        if (i>=($self)->size()) {
            PyErr_SetString(PyExc_TypeError, "Out of range");
            return std::nan("");
        }
        return (*($self))(i);
    }
}

// OpenMEEG::Matrix

%extend OpenMEEG::Matrix {
    Matrix(PyObject* o) {
        return new_OpenMEEG_Matrix(o);
    }

    PyObject* array() {
        const npy_intp ndims = 2;
        npy_intp* dims = new npy_intp[ndims];
        dims[0] = ($self)->nlin();
        dims[1] = ($self)->ncol();

        PyArrayObject* array = reinterpret_cast<PyArrayObject*>(PyArray_New(&PyArray_Type,ndims,dims,NPY_DOUBLE,NULL,
                                                                            static_cast<void*>(($self)->data()),0,NPY_ARRAY_F_CONTIGUOUS,NULL));
        return PyArray_Return(array);
    }

    void setvalue(unsigned int i, unsigned int j, double d) {
        (*($self))(i,j) = d;
    }

    double value(const unsigned int i,const unsigned int j) {
        if ((i>=($self)->nlin()) || (j>=($self)->ncol())) {
            PyErr_SetString(PyExc_TypeError,"Out of range");
            return std::nan("");
        }
        return (*($self))(i,j);
    }
}

// OpenMEEG::Mesh

%ignore OpenMEEG::Mesh::name(); // ignore non const name() method

%extend OpenMEEG::Mesh{

    Mesh(PyObject* py_v,PyObject* py_i,std::string name="") {
        if ((py_v==nullptr || !PyArray_Check(py_v)) ||
            (py_i==nullptr || !PyArray_Check(py_i)))
            return new Mesh();

        PyArrayObject* mat_v  = reinterpret_cast<PyArrayObject*>(PyArray_FromObject(py_v,NPY_DOUBLE,0,0));
        if (mat_v==nullptr) {
            PyErr_SetString(PyExc_TypeError,
                            "Matrix of vertices is not wellformed, returning an empty matrix instead.");
            return new Mesh();
        }

        const size_t nbdims_v = PyArray_NDIM(mat_v);
        if (nbdims_v!=2) {
            PyErr_SetString(PyExc_TypeError,
                            "Matrix of vertices requires an 2 dimensions array, returning an empty matrix instead.");
            return new Mesh();
        }
        const size_t nbVertices = PyArray_DIM(mat_v,0);
        const size_t nbcol      = PyArray_DIM(mat_v,1);

        // Deal with both np.dtype arrays of UNIT64 and INT64

        PyArrayObject *arr = NULL;
        PyArray_Descr *dtype = new PyArray_Descr();
        int ndim = 0;
        npy_intp dims[NPY_MAXDIMS];

        if ((PyArray_GetArrayParamsFromObject(py_i,NULL,1,&dtype,&ndim,&dims[0],&arr,NULL)<0 ) || (arr==NULL)) {
            PyErr_SetString(PyExc_TypeError,"Cannot get array parameters for triangles array");
            return new Mesh();
        }

        const int array_type = PyArray_TYPE(arr);

        if (array_type!=NPY_INT64 && array_type!=NPY_UINT64) {
            PyErr_SetString(PyExc_TypeError,"Wrong dtype for triangles array (only int64 or uint64 supported)");
            return new Mesh();
        }

        PyArrayObject* mat_i = reinterpret_cast<PyArrayObject*>(PyArray_FromObject(py_i,array_type,0,0));
        if (mat_i==nullptr) {
            PyErr_SetString(PyExc_TypeError,"Matrix of triangles is not wellformed, returning an empty matrix instead.");
            return new Mesh();
        }

        const size_t nbdims_i = PyArray_NDIM(mat_i);
        if (nbdims_i!=2) {
            PyErr_SetString(PyExc_TypeError,"Matrix of triangles requires an 2 dimensions array, returning an empty matrix instead.");
            return new Mesh();
        }

        const size_t nbTriangles  = PyArray_DIM(mat_i,0);
        const size_t TriangleSize = PyArray_DIM(mat_i,1);
        if (TriangleSize!=3) {
            PyErr_SetString(PyExc_TypeError,"Matrix of triangles requires exactly 3 columns, standing for indices of 3 vertices.");
            return new Mesh();
        }

        Mesh* newMesh = new Mesh(nbVertices,nbTriangles);
        for (int vi=0;vi<nbVertices;++vi) {
            const double x = *reinterpret_cast<double*>(PyArray_GETPTR2(mat_v,vi,0));
            const double y = *reinterpret_cast<double*>(PyArray_GETPTR2(mat_v,vi,1));
            const double z = *reinterpret_cast<double*>(PyArray_GETPTR2(mat_v,vi,2));
            newMesh->geometry().add_vertex(*new Vertex(x,y,z));
        }

        auto get_vertex = [=](PyArrayObject* mat,const int i,const int j) {
            const unsigned vi = *reinterpret_cast<unsigned*>(PyArray_GETPTR2(mat,i,j));
            if (vi>=nbVertices)
                throw vi;
            return newMesh->vertices()[vi];
        };

        for (int ti=0;ti<nbTriangles;++ti) {
            try {
                Vertex* v1 = get_vertex(mat_i,ti,0);
                Vertex* v2 = get_vertex(mat_i,ti,1);
                Vertex* v3 = get_vertex(mat_i,ti,2);
                newMesh->triangles().push_back(Triangle(v1,v2,v3));
            } catch(unsigned& ind) {
                //  TODO: Improve the error message to indicate the triangle and the index of vertex
                PyErr_SetString(PyExc_TypeError,"Triangle index out of range");
                delete newMesh;
                return new Mesh();
            }
        }

        newMesh->name() = name;
        return newMesh;
    }

    const char* __str__() {
        return ($self)->name().c_str();
    }
}

%extend OpenMEEG::Geometry {

    void import_meshes(PyObject* pylist) {
        if (pylist==nullptr || !PyList_Check(pylist)) {
            PyErr_SetString(PyExc_TypeError,"Wrong parameter to import_meshes");
            return;
        }
        std::cerr << "Executing import_meshes" << std::endl;
        const unsigned N = PyList_Size(pylist);
        OpenMEEG::Meshes meshes(N);
        for (unsigned i=0;i<N;++i) {
            PyObject* item = PyList_GetItem(pylist,i);
            void* ptr = 0;
            if (!SWIG_IsOK(SWIG_ConvertPtr(item,&ptr,SWIGTYPE_p_OpenMEEG__Mesh,SWIG_POINTER_EXCEPTION))) {
                PyErr_SetString(PyExc_TypeError, "Input object must be a list of Mesh.");
                return;
            }
            meshes.emplace_back(std::forward<Mesh>(*reinterpret_cast<OpenMEEG::Mesh*>(ptr)));
        }
        $self->import_meshes(meshes);
    }
}

// Input

%include <vect3.h>
%include <vertex.h>
%include <triangle.h>
%include <linop.h>
%include <vector.h>
%include <matrix.h>
%include <symmatrix.h>
%include <sparse_matrix.h>
%include <fast_sparse_matrix.h>
%include <geometry.h>
%include <GeometryIO.h>
%include <sensors.h>
%include <mesh.h>
%include <interface.h>
%include <domain.h>
%include <assemble.h>
%include <gain.h>
%include <forward.h>

%pythoncode "make_geometry.py"
