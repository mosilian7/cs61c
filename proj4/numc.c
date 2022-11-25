#include "numc.h"
#include <structmember.h>
#include <stdio.h>

PyTypeObject Matrix61cType;

/* Helper functions for initalization of matrices and vectors */

/*
 * Return a tuple given rows and cols
 */
PyObject *get_shape(int rows, int cols) {
  if (rows == 1 || cols == 1) {
    return PyTuple_Pack(1, PyLong_FromLong(rows * cols));
  } else {
    return PyTuple_Pack(2, PyLong_FromLong(rows), PyLong_FromLong(cols));
  }
}
/*
 * Matrix(rows, cols, low, high). Fill a matrix random double values
 */
int init_rand(PyObject *self, int rows, int cols, unsigned int seed, double low,
              double high) {
    matrix *new_mat;
    int alloc_failed = allocate_matrix(&new_mat, rows, cols);
    if (alloc_failed) return alloc_failed;
    rand_matrix(new_mat, seed, low, high);
    ((Matrix61c *)self)->mat = new_mat;
    ((Matrix61c *)self)->shape = get_shape(new_mat->rows, new_mat->cols);
    return 0;
}

/*
 * Matrix(rows, cols, val). Fill a matrix of dimension rows * cols with val
 */
int init_fill(PyObject *self, int rows, int cols, double val) {
    matrix *new_mat;
    int alloc_failed = allocate_matrix(&new_mat, rows, cols);
    if (alloc_failed)
        return alloc_failed;
    else {
        fill_matrix(new_mat, val);
        ((Matrix61c *)self)->mat = new_mat;
        ((Matrix61c *)self)->shape = get_shape(new_mat->rows, new_mat->cols);
    }
    return 0;
}

/*
 * Matrix(rows, cols, 1d_list). Fill a matrix with dimension rows * cols with 1d_list values
 */
int init_1d(PyObject *self, int rows, int cols, PyObject *lst) {
    if (rows * cols != PyList_Size(lst)) {
        PyErr_SetString(PyExc_ValueError, "Incorrect number of elements in list");
        return -1;
    }
    matrix *new_mat;
    int alloc_failed = allocate_matrix(&new_mat, rows, cols);
    if (alloc_failed) return alloc_failed;
    int count = 0;
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            set(new_mat, i, j, PyFloat_AsDouble(PyList_GetItem(lst, count)));
            count++;
        }
    }
    ((Matrix61c *)self)->mat = new_mat;
    ((Matrix61c *)self)->shape = get_shape(new_mat->rows, new_mat->cols);
    return 0;
}

/*
 * Matrix(2d_list). Fill a matrix with dimension len(2d_list) * len(2d_list[0])
 */
int init_2d(PyObject *self, PyObject *lst) {
    int rows = PyList_Size(lst);
    if (rows == 0) {
        PyErr_SetString(PyExc_ValueError,
                        "Cannot initialize numc.Matrix with an empty list");
        return -1;
    }
    int cols;
    if (!PyList_Check(PyList_GetItem(lst, 0))) {
        PyErr_SetString(PyExc_ValueError, "List values not valid");
        return -1;
    } else {
        cols = PyList_Size(PyList_GetItem(lst, 0));
    }
    for (int i = 0; i < rows; i++) {
        if (!PyList_Check(PyList_GetItem(lst, i)) ||
                PyList_Size(PyList_GetItem(lst, i)) != cols) {
            PyErr_SetString(PyExc_ValueError, "List values not valid");
            return -1;
        }
    }
    matrix *new_mat;
    int alloc_failed = allocate_matrix(&new_mat, rows, cols);
    if (alloc_failed) return alloc_failed;
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            set(new_mat, i, j,
                PyFloat_AsDouble(PyList_GetItem(PyList_GetItem(lst, i), j)));
        }
    }
    ((Matrix61c *)self)->mat = new_mat;
    ((Matrix61c *)self)->shape = get_shape(new_mat->rows, new_mat->cols);
    return 0;
}

/*
 * This deallocation function is called when reference count is 0
 */
void Matrix61c_dealloc(Matrix61c *self) {
    deallocate_matrix(self->mat);
    Py_TYPE(self)->tp_free(self);
}

/* For immutable types all initializations should take place in tp_new */
PyObject *Matrix61c_new(PyTypeObject *type, PyObject *args,
                        PyObject *kwds) {
    /* size of allocated memory is tp_basicsize + nitems*tp_itemsize*/
    Matrix61c *self = (Matrix61c *)type->tp_alloc(type, 0);
    return (PyObject *)self;
}

/*
 * This matrix61c type is mutable, so needs init function. Return 0 on success otherwise -1
 */
int Matrix61c_init(PyObject *self, PyObject *args, PyObject *kwds) {
    /* Generate random matrices */
    if (kwds != NULL) {
        PyObject *rand = PyDict_GetItemString(kwds, "rand");
        if (!rand) {
            PyErr_SetString(PyExc_TypeError, "Invalid arguments");
            return -1;
        }
        if (!PyBool_Check(rand)) {
            PyErr_SetString(PyExc_TypeError, "Invalid arguments");
            return -1;
        }
        if (rand != Py_True) {
            PyErr_SetString(PyExc_TypeError, "Invalid arguments");
            return -1;
        }

        PyObject *low = PyDict_GetItemString(kwds, "low");
        PyObject *high = PyDict_GetItemString(kwds, "high");
        PyObject *seed = PyDict_GetItemString(kwds, "seed");
        double double_low = 0;
        double double_high = 1;
        unsigned int unsigned_seed = 0;

        if (low) {
            if (PyFloat_Check(low)) {
                double_low = PyFloat_AsDouble(low);
            } else if (PyLong_Check(low)) {
                double_low = PyLong_AsLong(low);
            }
        }

        if (high) {
            if (PyFloat_Check(high)) {
                double_high = PyFloat_AsDouble(high);
            } else if (PyLong_Check(high)) {
                double_high = PyLong_AsLong(high);
            }
        }

        if (double_low >= double_high) {
            PyErr_SetString(PyExc_TypeError, "Invalid arguments");
            return -1;
        }

        // Set seed if argument exists
        if (seed) {
            if (PyLong_Check(seed)) {
                unsigned_seed = PyLong_AsUnsignedLong(seed);
            }
        }

        PyObject *rows = NULL;
        PyObject *cols = NULL;
        if (PyArg_UnpackTuple(args, "args", 2, 2, &rows, &cols)) {
            if (rows && cols && PyLong_Check(rows) && PyLong_Check(cols)) {
                return init_rand(self, PyLong_AsLong(rows), PyLong_AsLong(cols), unsigned_seed, double_low,
                                 double_high);
            }
        } else {
            PyErr_SetString(PyExc_TypeError, "Invalid arguments");
            return -1;
        }
    }
    PyObject *arg1 = NULL;
    PyObject *arg2 = NULL;
    PyObject *arg3 = NULL;
    if (PyArg_UnpackTuple(args, "args", 1, 3, &arg1, &arg2, &arg3)) {
        /* arguments are (rows, cols, val) */
        if (arg1 && arg2 && arg3 && PyLong_Check(arg1) && PyLong_Check(arg2) && (PyLong_Check(arg3)
                || PyFloat_Check(arg3))) {
            if (PyLong_Check(arg3)) {
                return init_fill(self, PyLong_AsLong(arg1), PyLong_AsLong(arg2), PyLong_AsLong(arg3));
            } else
                return init_fill(self, PyLong_AsLong(arg1), PyLong_AsLong(arg2), PyFloat_AsDouble(arg3));
        } else if (arg1 && arg2 && arg3 && PyLong_Check(arg1) && PyLong_Check(arg2) && PyList_Check(arg3)) {
            /* Matrix(rows, cols, 1D list) */
            return init_1d(self, PyLong_AsLong(arg1), PyLong_AsLong(arg2), arg3);
        } else if (arg1 && PyList_Check(arg1) && arg2 == NULL && arg3 == NULL) {
            /* Matrix(rows, cols, 1D list) */
            return init_2d(self, arg1);
        } else if (arg1 && arg2 && PyLong_Check(arg1) && PyLong_Check(arg2) && arg3 == NULL) {
            /* Matrix(rows, cols, 1D list) */
            return init_fill(self, PyLong_AsLong(arg1), PyLong_AsLong(arg2), 0);
        } else {
            PyErr_SetString(PyExc_TypeError, "Invalid arguments");
            return -1;
        }
    } else {
        PyErr_SetString(PyExc_TypeError, "Invalid arguments");
        return -1;
    }
}

/*
 * List of lists representations for matrices
 */
PyObject *Matrix61c_to_list(Matrix61c *self) {
    int rows = self->mat->rows;
    int cols = self->mat->cols;
    PyObject *py_lst = NULL;
    if (self->mat->is_1d) {  // If 1D matrix, print as a single list
        py_lst = PyList_New(rows * cols);
        int count = 0;
        for (int i = 0; i < rows; i++) {
            for (int j = 0; j < cols; j++) {
                PyList_SetItem(py_lst, count, PyFloat_FromDouble(get(self->mat, i, j)));
                count++;
            }
        }
    } else {  // if 2D, print as nested list
        py_lst = PyList_New(rows);
        for (int i = 0; i < rows; i++) {
            PyList_SetItem(py_lst, i, PyList_New(cols));
            PyObject *curr_row = PyList_GetItem(py_lst, i);
            for (int j = 0; j < cols; j++) {
                PyList_SetItem(curr_row, j, PyFloat_FromDouble(get(self->mat, i, j)));
            }
        }
    }
    return py_lst;
}

PyObject *Matrix61c_class_to_list(Matrix61c *self, PyObject *args) {
    PyObject *mat = NULL;
    if (PyArg_UnpackTuple(args, "args", 1, 1, &mat)) {
        if (!PyObject_TypeCheck(mat, &Matrix61cType)) {
            PyErr_SetString(PyExc_TypeError, "Argument must of type numc.Matrix!");
            return NULL;
        }
        Matrix61c* mat61c = (Matrix61c*)mat;
        return Matrix61c_to_list(mat61c);
    } else {
        PyErr_SetString(PyExc_TypeError, "Invalid arguments");
        return NULL;
    }
}

/*
 * Add class methods
 */
PyMethodDef Matrix61c_class_methods[] = {
    {"to_list", (PyCFunction)Matrix61c_class_to_list, METH_VARARGS, "Returns a list representation of numc.Matrix"},
    {NULL, NULL, 0, NULL}
};

/*
 * Matrix61c string representation. For printing purposes.
 */
PyObject *Matrix61c_repr(PyObject *self) {
    PyObject *py_lst = Matrix61c_to_list((Matrix61c *)self);
    return PyObject_Repr(py_lst);
}

/* NUMBER METHODS */

/*
 * Add the second numc.Matrix (Matrix61c) object to the first one. The first operand is
 * self, and the second operand can be obtained by casting `args`.
 */
PyObject *Matrix61c_add(Matrix61c* self, PyObject* args) {
    /* TODO: YOUR CODE HERE */
    if (!PyObject_TypeCheck(args, &Matrix61cType)) {
        PyErr_SetString(PyExc_TypeError, "add: invalid argument type");
        return NULL;
    }
    if (self->mat->rows != ((Matrix61c*)args)->mat->rows || self->mat->cols != ((Matrix61c*)args)->mat->cols) {
        PyErr_SetString(PyExc_ValueError, "add: invalid dimension");
        return NULL;
    }
    Matrix61c* result = (Matrix61c*) Matrix61c_new(&Matrix61cType, NULL, NULL);
    
    int result_rows = self->mat->rows;
    int result_cols = self->mat->cols;
    result->shape = get_shape(result_rows, result_cols);

    allocate_matrix(&(result->mat), result_rows, result_cols);
    add_matrix(result->mat, self->mat, ((Matrix61c*)args)->mat);
    return (PyObject*)result;
}

/*
 * Substract the second numc.Matrix (Matrix61c) object from the first one. The first operand is
 * self, and the second operand can be obtained by casting `args`.
 */
PyObject *Matrix61c_sub(Matrix61c* self, PyObject* args) {
    /* TODO: YOUR CODE HERE */
    if (!PyObject_TypeCheck(args, &Matrix61cType)) {
        PyErr_SetString(PyExc_TypeError, "sub: invalid argument type");
        return NULL;
    }
    if (self->mat->rows != ((Matrix61c*)args)->mat->rows || self->mat->cols != ((Matrix61c*)args)->mat->cols) {
        PyErr_SetString(PyExc_ValueError, "sub: invalid dimension");
        return NULL;
    }
    Matrix61c* result = (Matrix61c*) Matrix61c_new(&Matrix61cType, NULL, NULL);
    
    int result_rows = self->mat->rows;
    int result_cols = self->mat->cols;
    result->shape = get_shape(result_rows, result_cols);

    allocate_matrix(&(result->mat), result_rows, result_cols);
    sub_matrix(result->mat, self->mat, ((Matrix61c*)args)->mat);
    return (PyObject*)result;
}


/*
 * NOT element-wise multiplication. The first operand is self, and the second operand
 * can be obtained by casting `args`.
 */
PyObject *Matrix61c_multiply(Matrix61c* self, PyObject *args) {
    /* TODO: YOUR CODE HERE */
    if (!PyObject_TypeCheck(args, &Matrix61cType)) {
        PyErr_SetString(PyExc_TypeError, "mul: invalid argument type");
        return NULL;
    }
    if (self->mat->cols != ((Matrix61c*)args)->mat->rows) {
        PyErr_SetString(PyExc_ValueError, "mul: invalid dimension");
        return NULL;
    }
    Matrix61c* result = (Matrix61c*) Matrix61c_new(&Matrix61cType, NULL, NULL);
    
    int result_rows = self->mat->rows;
    int result_cols = ((Matrix61c*)args)->mat->cols;
    result->shape = get_shape(result_rows, result_cols);

    allocate_matrix(&(result->mat), result_rows, result_cols);
    mul_matrix(result->mat, self->mat, ((Matrix61c*)args)->mat);
    return (PyObject*)result;
}

/*
 * Negates the given numc.Matrix.
 */
PyObject *Matrix61c_neg(Matrix61c* self) {
    /* TODO: YOUR CODE HERE */
    Matrix61c* result = (Matrix61c*) Matrix61c_new(&Matrix61cType, NULL, NULL);
    
    int result_rows = self->mat->rows;
    int result_cols = self->mat->cols;
    result->shape = get_shape(result_rows, result_cols);

    allocate_matrix(&(result->mat), result_rows, result_cols);
    neg_matrix(result->mat, self->mat);
    return (PyObject*)result;
}

/*
 * Take the element-wise absolute value of this numc.Matrix.
 */
PyObject *Matrix61c_abs(Matrix61c *self) {
    /* TODO: YOUR CODE HERE */
    Matrix61c* result = (Matrix61c*) Matrix61c_new(&Matrix61cType, NULL, NULL);
    
    int result_rows = self->mat->rows;
    int result_cols = self->mat->cols;
    result->shape = get_shape(result_rows, result_cols);

    allocate_matrix(&(result->mat), result_rows, result_cols);
    abs_matrix(result->mat, self->mat);
    return (PyObject*)result;
}

/*
 * Raise numc.Matrix (Matrix61c) to the `pow`th power. You can ignore the argument `optional`.
 */
PyObject *Matrix61c_pow(Matrix61c *self, PyObject *pow, PyObject *optional) {
    /* TODO: YOUR CODE HERE */
    if (!PyObject_TypeCheck(pow, &PyLong_Type)) {
        PyErr_SetString(PyExc_TypeError, "pow: invalid argument type");
        return NULL;
    }
    if (PyLong_AsLong(pow) < 0) {
        PyErr_SetString(PyExc_ValueError, "pow: invalid argument");
        return NULL;
    }
    if (self->mat->cols != self->mat->rows) {
        PyErr_SetString(PyExc_ValueError, "pow: invalid dimension");
        return NULL;
    }

    Matrix61c* result = (Matrix61c*) Matrix61c_new(&Matrix61cType, NULL, NULL);

    int result_rows = self->mat->rows;
    int result_cols = self->mat->cols;
    result->shape = get_shape(result_rows, result_cols);

    allocate_matrix(&(result->mat), result_rows, result_cols);
    pow_matrix(result->mat, self->mat, (int)PyLong_AsLong(pow));
    return (PyObject*)result;
}

/*
 * Create a PyNumberMethods struct for overloading operators with all the number methods you have
 * define. You might find this link helpful: https://docs.python.org/3.6/c-api/typeobj.html
 */
PyNumberMethods Matrix61c_as_number = {
    /* TODO: YOUR CODE HERE */
    .nb_add = (binaryfunc) Matrix61c_add,
    .nb_subtract = (binaryfunc) Matrix61c_sub,
    .nb_multiply = (binaryfunc) Matrix61c_multiply,
    .nb_power = (binaryfunc) Matrix61c_pow,
    .nb_negative = (binaryfunc) Matrix61c_neg,
    .nb_absolute = (binaryfunc) Matrix61c_abs,
};


/* INSTANCE METHODS */

/*
 * Given a numc.Matrix self, parse `args` to (int) row, (int) col, and (double/int) val.
 * Return None in Python (this is different from returning null).
 */
PyObject *Matrix61c_set_value(Matrix61c *self, PyObject* args) {
    /* TODO: YOUR CODE HERE */
    PyObject* row = NULL;
    PyObject* col = NULL;
    PyObject* val = NULL;

    if (PyArg_UnpackTuple(args, "args", 3, 3, &row, &col, &val)) {
        if ((!PyObject_TypeCheck(row, &PyLong_Type)) ||
            (!PyObject_TypeCheck(col, &PyLong_Type)) ||
            (!(PyObject_TypeCheck(val, &PyLong_Type) || PyObject_TypeCheck(val, &PyFloat_Type)))) {
                PyErr_SetString(PyExc_TypeError, "set: invalid argument type");
                return NULL;
            }
        int row_val = (int) PyLong_AsLong(row);
        int col_val = (int) PyLong_AsLong(col);
        double val_val = PyObject_TypeCheck(val, &PyLong_Type) ? 
                        (double)PyLong_AsLong(val) : PyFloat_AsDouble(val);
        if (row_val < 0 || col_val < 0 || row_val >= self->mat->rows || col_val >= self->mat->cols) {
            PyErr_SetString(PyExc_IndexError, "set: index out of bound");
            return NULL;
        }
        set(self->mat, row_val, col_val, val_val);
        return Py_None;
    } else {
        PyErr_SetString(PyExc_TypeError, "set: invalid argument count");
        return NULL;
    }
    
}

/*
 * Given a numc.Matrix `self`, parse `args` to (int) row and (int) col.
 * Return the value at the `row`th row and `col`th column, which is a Python
 * float/int.
 */
PyObject *Matrix61c_get_value(Matrix61c *self, PyObject* args) {
    /* TODO: YOUR CODE HERE */
    PyObject* row = NULL;
    PyObject* col = NULL;

    if (PyArg_UnpackTuple(args, "args", 2, 2, &row, &col)) {
        if ((!PyObject_TypeCheck(row, &PyLong_Type)) || (!PyObject_TypeCheck(col, &PyLong_Type))) {
                PyErr_SetString(PyExc_TypeError, "get: invalid argument type");
                return NULL;
            }
        int row_val = (int) PyLong_AsLong(row);
        int col_val = (int) PyLong_AsLong(col);
        double val;
        if (row_val < 0 || col_val < 0 || row_val >= self->mat->rows || col_val >= self->mat->cols) {
            PyErr_SetString(PyExc_IndexError, "get: index out of bound");
            return NULL;
        }
        val = get(self->mat, row_val, col_val);
        return PyFloat_FromDouble(val);
    } else {
        PyErr_SetString(PyExc_TypeError, "get: invalid argument count");
        return NULL;
    }
}

/*
 * Create an array of PyMethodDef structs to hold the instance methods.
 * Name the python function corresponding to Matrix61c_get_value as "get" and Matrix61c_set_value
 * as "set"
 * You might find this link helpful: https://docs.python.org/3.6/c-api/structures.html
 */
PyMethodDef Matrix61c_methods[] = {
    /* TODO: YOUR CODE HERE */
    {"set", Matrix61c_set_value, METH_VARARGS, "Set self’s entry at the ith row and jth column to val."},
    {"get", Matrix61c_get_value, METH_VARARGS, "Returns the entry at the ith row and jth column."},
    {NULL, NULL, 0, NULL}
};

/* INDEXING */
/* Helper methods */
int _locate_ref_1d(PyObject* key, int length, int* offset, int* range) {
    if (PyObject_TypeCheck(key, &PyLong_Type)) {
        int int_key = PyLong_AsLong(key);
        if (int_key < 0 || int_key >= length) {
            return 1; //PyErr_SetString(PyExc_IndexError, "_locate_ref_1d: index out of bound");
        }
        *offset = int_key;
        *range = 1;
        return 0;
    } else if (PyObject_TypeCheck(key, &PySlice_Type)) {
        Py_ssize_t start=0, stop=0, step=0, slicelength=0;
        if (PySlice_GetIndicesEx(key, (Py_ssize_t)length, &start, &stop, &step, &slicelength) == 0) {
            //printf("start: %ld, stop: %ld, step: %ld, slicelength: %ld\n", start, stop, step, slicelength);
            if ((long)step != 1 || (long)slicelength < 1) {
                return 2; //PyErr_SetString(PyExc_ValueError, "_locate_ref_1d: invalid slice");
            }
            *offset = start;
            *range = slicelength;
            return 0;
        } else {
            return 3; //PyErr_SetString(PyExc_RuntimeError, "_locate_ref_1d: PySlice_GetIndicesEx failed");
        }
    } else { 
        return 4; //PyErr_SetString(PyExc_TypeError, "_locate_ref_1d: indexing should be operated by integer or slice");
    }
}

int _locate_ref(Matrix61c* self, PyObject* key, int* row_offset, int* col_offset, int* rows, int* cols) {
    /* TODO: find the location */
    int key_is_tuple = PyObject_TypeCheck(key, &PyTuple_Type);
    if (self->mat->is_1d) {
        if (key_is_tuple) {
            return 5; //PyErr_SetString(PyExc_TypeError, "_locate_ref: 1d matrix can only be indexed by integer or slice");
        }
        if (self->mat->rows == 1) {
            int error_code = _locate_ref_1d(key, self->mat->cols, col_offset, cols);
            *row_offset = 0;
            *rows = self->mat->rows;
            return error_code;
        }
    }
    if (key_is_tuple) {
        PyObject *row_key, *col_key;
        if (PyArg_UnpackTuple(key, "key", 2, 2, &row_key, &col_key)) {
            int error_code_row = _locate_ref_1d(row_key, self->mat->rows, row_offset, rows);
            int error_code_col = _locate_ref_1d(col_key, self->mat->cols, col_offset, cols);
            return (error_code_row == 0) ? error_code_col : error_code_row;
        } else {
            return 6; //PyErr_SetString(PyExc_TypeError, "_locate_ref: invalid argument count");
        }
    } else {
        int error_code = _locate_ref_1d(key, self->mat->rows, row_offset, rows);
        *col_offset = 0;
        *cols = self->mat->cols;
        return error_code;
    }
}

/*
 * Given a numc.Matrix `self`, index into it with `key`. Return the indexed result.
 */
PyObject *Matrix61c_subscript(Matrix61c* self, PyObject* key) {
    /* TODO: YOUR CODE HERE */
    /* NEED MORE ABSTRACTION!!! */
    int row_offset=0, col_offset=0, rows=0, cols=0;
    int error_code = _locate_ref(self, key, &row_offset, &col_offset, &rows, &cols);
    if (error_code == 0) {
        if (rows == 1 && cols == 1) {
            return PyFloat_FromDouble(self->mat->data[row_offset][col_offset]);
        } else {
            Matrix61c* result = (Matrix61c*) Matrix61c_new(&Matrix61cType, NULL, NULL);
            result->shape = get_shape(rows, cols);
            allocate_matrix_ref(&(result->mat), self->mat, row_offset, col_offset, rows, cols);
            return (PyObject*)result;
        }
    } else {
        switch (error_code) {
            case 1: PyErr_SetString(PyExc_IndexError, "_locate_ref_1d: index out of bound"); break;
            case 2: PyErr_SetString(PyExc_ValueError, "_locate_ref_1d: invalid slice"); break;
            case 3: PyErr_SetString(PyExc_RuntimeError, "_locate_ref_1d: PySlice_GetIndicesEx failed"); break;
            case 4: PyErr_SetString(PyExc_TypeError, "_locate_ref_1d: indexing should be operated by integer or slice"); break;
            case 5: PyErr_SetString(PyExc_TypeError, "_locate_ref: 1d matrix can only be indexed by integer or slice"); break;
            case 6: PyErr_SetString(PyExc_TypeError, "_locate_ref: invalid argument count"); break;
            default: PyErr_SetString(PyExc_RuntimeError, "subscript: error occurs when calling _locate_ref");
        }
        return NULL;
    }
}

/* Helper method */
int _set_element(Matrix61c* self, int row, int col, PyObject* element) {
    double element_val;
    if (PyObject_TypeCheck(element, &PyFloat_Type)) {
        element_val = PyFloat_AsDouble(element);
    } else if (PyObject_TypeCheck(element, &PyLong_Type)) {
        element_val = (double)PyLong_AsLong(element);
    } else {
        return 1; //PyErr_SetString(PyExc_TypeError, "_set_element: invalid element type ");
    }
    set(self->mat, row, col, element_val);
    return 0;
}

/*
 * Given a numc.Matrix `self`, index into it with `key`, and set the indexed result to `v`.
 */
int Matrix61c_set_subscript(Matrix61c* self, PyObject *key, PyObject *v) {
    /* TODO: YOUR CODE HERE */
    int row_offset=0, col_offset=0, rows=0, cols=0;
    int locate_error_code = _locate_ref(self, key, &row_offset, &col_offset, &rows, &cols);
    int set_error_code=0;
    if (locate_error_code == 0) {
        if (rows == 1 && cols == 1) {
            set_error_code = _set_element(self, row_offset, col_offset, v);
            goto HANDLE_SET_ERROR;
        } else if (rows == 1 || cols == 1) {
            if (PyObject_TypeCheck(v, &PyList_Type)) {
                Py_ssize_t list_len = PyList_Size(v);
                if (list_len != rows * cols) {
                    PyErr_SetString(PyExc_ValueError, "set_subscript: invalid list size ");
                    return -1;
                }
                PyObject* element;
                if (rows == 1) {
                    for (int i = 0; i < list_len; i += 1) {
                        element = PyList_GetItem(v, i);
                        set_error_code = _set_element(self, row_offset, col_offset + i, element); 
                        if (set_error_code != 0)
                            goto HANDLE_SET_ERROR;
                    }
                    goto HANDLE_SET_ERROR;
                } else {
                    for (int i = 0; i < list_len; i += 1) {
                        element = PyList_GetItem(v, i);
                        set_error_code = _set_element(self, row_offset + i, col_offset, element); 
                        if (set_error_code != 0)
                            goto HANDLE_SET_ERROR;
                    }
                    goto HANDLE_SET_ERROR;
                }
            } else {
                PyErr_SetString(PyExc_TypeError, "set_subscript: value should be a 1d list ");
                return -1;
            }
        } else {
            if (PyObject_TypeCheck(v, &PyList_Type)) {
                Py_ssize_t list_rows = PyList_Size(v);
                if (list_rows != rows) {
                    PyErr_SetString(PyExc_ValueError, "set_subscript: invalid list size ");
                    return -1;
                }
                PyObject* row_element;
                for (int i = 0; i < list_rows; i += 1) {
                    row_element = PyList_GetItem(v, i);
                    if (!PyObject_TypeCheck(row_element, &PyList_Type)) {
                        PyErr_SetString(PyExc_TypeError, "set_subscript: value should be a 2d list ");
                        return -1;
                    }
                    Py_ssize_t list_cols = PyList_Size(row_element);
                    if (list_cols != cols) {
                        PyErr_SetString(PyExc_ValueError, "set_subscript: invalid list size ");
                        return -1;
                    }
                    PyObject* element;
                    for (int j = 0; j < list_cols; j += 1) {
                        element = PyList_GetItem(row_element, j);
                        set_error_code = _set_element(self, row_offset + i, col_offset + j, element); 
                        if (set_error_code != 0)
                            goto HANDLE_SET_ERROR;
                    }
                }
                goto HANDLE_SET_ERROR;
            }
        }
    } else {
        switch (locate_error_code) {
            case 1: PyErr_SetString(PyExc_IndexError, "_locate_ref_1d: index out of bound"); break;
            case 2: PyErr_SetString(PyExc_ValueError, "_locate_ref_1d: invalid slice"); break;
            case 3: PyErr_SetString(PyExc_RuntimeError, "_locate_ref_1d: PySlice_GetIndicesEx failed"); break;
            case 4: PyErr_SetString(PyExc_TypeError, "_locate_ref_1d: indexing should be operated by integer or slice"); break;
            case 5: PyErr_SetString(PyExc_TypeError, "_locate_ref: 1d matrix can only be indexed by integer or slice"); break;
            case 6: PyErr_SetString(PyExc_TypeError, "_locate_ref: invalid argument count"); break;
            default: PyErr_SetString(PyExc_RuntimeError, "subscript: error occurs when calling _locate_ref");
        }
        return -1;
    }

    HANDLE_SET_ERROR: {
        switch (set_error_code) {
            case 0: return 0;
            case 1: PyErr_SetString(PyExc_ValueError, "_set_element: invalid list element type "); break;
            default: PyErr_SetString(PyExc_RuntimeError, "set_subscript: _set_element failed ");
        }
        return -1;
    }
}

PyMappingMethods Matrix61c_mapping = {
    NULL,
    (binaryfunc) Matrix61c_subscript,
    (objobjargproc) Matrix61c_set_subscript,
};

/* INSTANCE ATTRIBUTES*/
PyMemberDef Matrix61c_members[] = {
    {
        "shape", T_OBJECT_EX, offsetof(Matrix61c, shape), 0,
        "(rows, cols)"
    },
    {NULL}  /* Sentinel */
};

PyTypeObject Matrix61cType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "numc.Matrix",
    .tp_basicsize = sizeof(Matrix61c),
    .tp_dealloc = (destructor)Matrix61c_dealloc,
    .tp_repr = (reprfunc)Matrix61c_repr,
    .tp_as_number = &Matrix61c_as_number,
    .tp_flags = Py_TPFLAGS_DEFAULT |
    Py_TPFLAGS_BASETYPE,
    .tp_doc = "numc.Matrix objects",
    .tp_methods = Matrix61c_methods,
    .tp_members = Matrix61c_members,
    .tp_as_mapping = &Matrix61c_mapping,
    .tp_init = (initproc)Matrix61c_init,
    .tp_new = Matrix61c_new
};


struct PyModuleDef numcmodule = {
    PyModuleDef_HEAD_INIT,
    "numc",
    "Numc matrix operations",
    -1,
    Matrix61c_class_methods
};

/* Initialize the numc module */
PyMODINIT_FUNC PyInit_numc(void) {
    PyObject* m;

    if (PyType_Ready(&Matrix61cType) < 0)
        return NULL;

    m = PyModule_Create(&numcmodule);
    if (m == NULL)
        return NULL;

    Py_INCREF(&Matrix61cType);
    PyModule_AddObject(m, "Matrix", (PyObject *)&Matrix61cType);
    printf("CS61C Fall 2020 Project 4: numc imported!\n");
    fflush(stdout);
    return m;
}