/**
    simhash functions for python

    * Convert hashes to fingerprints
    * Calculate hamming distances
    * Find hashes within a certain hamming distance
*/
#include <Python.h>
#include "fnv.h"

#define HASHBITS (sizeof(PY_LONG_LONG) * 8)

#define ALMOST_0 -1.0e-7

/*TODO: make hamdist work outside GCC, sample code below (slower) */
#define hamdist(x, y) __builtin_popcountll((x)^(y))

#if 0
static int hamdist(PY_LONG_LONG x, PY_LONG_LONG y)
{
    int dist = 0;
    PY_LONG_LONG val;
    for (val = x ^ y; val; val &= val - 1)
        ++dist;
    return dist;
}
#endif

static PyObject *pyhammdist(PyObject *self, PyObject *args)
{
    PY_LONG_LONG x, y, dist;
    if(!PyArg_ParseTuple(args, "LL", &x, &y))
        return NULL;
    dist = hamdist(x, y);
    return PyLong_FromLongLong(dist);
}

static PyObject *fingerprint(PyObject *self, PyObject *args)
{
    PyObject *seq;
    Py_ssize_t i;
    int sums[HASHBITS] = {0};

    if(!PyArg_ParseTuple(args, "O", &seq))
        return NULL;

    Py_ssize_t seq_length = PySequence_Length(seq);
    if (seq_length < 0)
        return NULL; /* Has no length */

    for (i = 0; i < seq_length; ++i) {
        Py_ssize_t index;
        PyObject *item = PySequence_GetItem(seq, i);
        if (item == NULL)
            return NULL;
        PY_LONG_LONG hashval = PyLong_AsLongLong(item);
        if (hashval == -1 && PyErr_Occurred() != NULL) {
            Py_DECREF(item);
            return NULL;
        }
        Py_DECREF(item);
        for (index=0; index < HASHBITS; ++index){
            if (hashval & 1)
                ++sums[index];
            else
                --sums[index];
            hashval >>= 1;
        }
    }
    /* build output */
    PY_LONG_LONG result = 0;
    for (i=0; i < HASHBITS; ++i){
        int nextval = sums[i];
        if (nextval >= 0)
            result |= 1;
        result <<= 1;
    }
    return PyLong_FromLongLong(result);
}

static PyObject *weighted_fingerprint(PyObject *self, PyObject *args)
{
    PyObject *seq;
    Py_ssize_t i;
    float sums[HASHBITS] = {0.0};

    if(!PyArg_ParseTuple(args, "O", &seq))
        return NULL;

    Py_ssize_t seq_length = PySequence_Length(seq);
    if (seq_length < 0)
        return NULL; /* Has no length */

    for (i = 0; i < seq_length; ++i) {
        Py_ssize_t index;
        PY_LONG_LONG hashval;
        float weight;
        PyObject *item = PySequence_GetItem(seq, i);
        if (item == NULL)
            return NULL;
        if (!PyArg_ParseTuple(item, "Lf", &hashval, &weight)){
            PyErr_SetString(PyExc_TypeError,
                "invalid item, expecting tuple of long long and float");
            Py_DECREF(item);
            return NULL;
        }
        Py_DECREF(item);
        for (index=0; index < HASHBITS; ++index){
            if (hashval & 1)
                sums[index] += weight;
            else
                sums[index] -= weight;
            hashval >>= 1;
        }
    }
    /* build output */
    PY_LONG_LONG result = 0;
    for (i=0; i < HASHBITS; ++i){
        /* values often end up close to 0, but not precisely 0, due to FP rounding */
        result |= ((PY_LONG_LONG)(sums[i] > ALMOST_0)) << i;
    }
    return PyLong_FromLongLong(result);
}

struct hashrec {
    PY_LONG_LONG hash;
    Py_ssize_t offset;
};

static int cmp_hashrec(const void *a, const void *b) {
    return ((const struct hashrec *) a)->hash -
        ((const struct hashrec *) b)->hash;
}

static PyObject *similar_indices(PyObject *self, PyObject *args)
{
    PyObject *seq;
    Py_ssize_t i;
    int keybits, maxbitdifference, rotate_bits;
    if(!PyArg_ParseTuple(args, "Oiii", &seq, &keybits, &maxbitdifference,
            &rotate_bits))
        return NULL;
    Py_ssize_t seq_length = PySequence_Length(seq);
    if (seq_length < 0)
        return NULL; /* Has no length */

    /* copy sequence to an array of struct hashrec, rotating as appropriate */
    struct hashrec* simvals = malloc(seq_length * sizeof(struct hashrec));
    for (i=0; i < seq_length; ++i){
        PyObject *item = PySequence_GetItem(seq, i);
        if (item == NULL)
            return NULL;
        PY_LONG_LONG hashval = PyLong_AsLongLong(item);
        if (hashval == -1 && PyErr_Occurred() != NULL) {
            Py_DECREF(item);
            return NULL;
        }
        simvals[i].hash = (hashval << rotate_bits) |
            ((unsigned PY_LONG_LONG)hashval >> (HASHBITS - rotate_bits));
        simvals[i].offset = i;
    }

    /* sort array by hash */
    qsort(simvals, seq_length, sizeof(struct hashrec), cmp_hashrec);

    /* enumerate similar pairs and add to output list */
    PY_LONG_LONG mask = ((1LL << rotate_bits) - 1) << (HASHBITS - rotate_bits);
    PyObject *outlist = PyList_New(0);
    for (i=0; i < seq_length - 1; ++i) {
        Py_ssize_t j, ii = simvals[i].offset;
        PY_LONG_LONG ihashval = simvals[i].hash;
        PY_LONG_LONG key = ihashval & mask;
        for (j = i+1; j < seq_length; ++j) {
            PY_LONG_LONG jhashval = simvals[j].hash;
            if ((jhashval & mask) != key)
                break;
            if (hamdist(ihashval, jhashval) <= maxbitdifference) {
                PyObject *pair;
                if (ii < simvals[j].offset)
                    pair = Py_BuildValue("(nn)", ii, simvals[j].offset);
                else
                    pair = Py_BuildValue("(nn)", simvals[j].offset, ii);
                if (PyList_Append(outlist, pair) == -1)
                    return NULL;
                Py_DECREF(pair);
            }
        }
    }
    free(simvals);
    return outlist;
}

static PyObject *hash(PyObject *self, PyObject *args)
{
    const char *str;
    uint64_t hash;

    if (PyArg_ParseTuple(args, "s", &str)) {
        hash = fnv_str(str);
        return PyLong_FromLongLong(hash);
    }

    return NULL;
}

static PyMethodDef functions[] = {
    {"weighted_fingerprint", weighted_fingerprint, METH_VARARGS,
        "generate a fingerprint from a sequence of (long long, weight) tuples"},
    {"fingerprint", fingerprint, METH_VARARGS,
        "generate a fingerprint from a sequence of long long integers"},
    {"hamming_distance", pyhammdist, METH_VARARGS,
        "calculate the number of bits that differ between 2 long long integers"},
    {"similar_indices", similar_indices, METH_VARARGS,
        "returns pairs of indices that differ less than N bits"},
    {"fnvhash", hash, METH_VARARGS,
        "generates a (FNV-1a) hash from a string"},

    {NULL, NULL, 0, NULL}
};

#if PY_MAJOR_VERSION >= 3
  static struct PyModuleDef moduledef = {
    PyModuleDef_HEAD_INIT,
    "_simhash", /* m_name */
    "simhashing for python",      /* m_doc */
    -1,                  /* m_size */
    functions,    /* m_methods */
    NULL,                /* m_reload */
    NULL,                /* m_traverse */
    NULL,                /* m_clear */
    NULL,                /* m_free */
  };
#endif

#if PY_MAJOR_VERSION < 3
    PyMODINIT_FUNC
    init_simhash(void)
    {
        Py_InitModule3("_simhash", functions, "simhashing for python");
    }
#else
    PyMODINIT_FUNC
    PyInit__simhash(void)
    {
        return PyModule_Create(&moduledef);
    }
#endif
