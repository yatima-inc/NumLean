/*
    Copyright (c) 2021 Arthur Paulino. All rights reserved.
    Released under Apache 2.0 license as described in the file LICENSE.
    Authors: Arthur Paulino
*/

#include <lean/lean.h>
#include <cstring>
#include <stdio.h>

#define internal inline static
#define external extern "C" LEAN_EXPORT
#define l_arg b_lean_obj_arg
#define l_res lean_obj_res
#define l_obj lean_object

typedef struct nl_matrix {
    uint64_t n_rows = 0;
    uint64_t n_cols = 0;
    uint64_t length = 0;
    double*    data = NULL;
} nl_matrix;

static lean_external_class* g_nl_matrix_external_class = NULL;

internal void nl_matrix_finalizer(void* m_) {
    nl_matrix* m = (nl_matrix*) m_;
    if (m->data) {
        free(m->data);
    }
    free(m);
}

internal void noop_foreach(void* mod, l_arg fn) {}

internal nl_matrix* nl_matrix_alloc(uint64_t n_rows, uint64_t n_cols) {
    nl_matrix* m = (nl_matrix*) malloc(sizeof(nl_matrix));
    if (!m) {
        return NULL;
    }
    m->data = (double*) malloc(n_rows * n_cols * sizeof(double));
    if (!m->data) {
        return NULL;
    }
    m->n_rows = n_rows;
    m->n_cols = n_cols;
    m->length = n_rows * n_cols;
    return m;
}

internal nl_matrix* nl_matrix_copy(nl_matrix* m) {
    nl_matrix* m_ = nl_matrix_alloc(m->n_rows, m->n_cols);
    if (!m_) {
        return NULL;
    }
    memcpy(m_->data, m->data, m->length * sizeof(double));
    return m;
}

internal l_res make_error(const char* err_msg) {
    return lean_mk_io_user_error(lean_mk_io_user_error(lean_mk_string(err_msg)));
}

internal double get_val(nl_matrix* m, uint64_t i, uint64_t j) {
    return *(m->data + (j + i * m->n_cols) * sizeof(double));
}

internal void set_val(nl_matrix* m, uint64_t i, uint64_t j, double v) {
    *(m->data + (j + i * m->n_cols) * sizeof(double)) = v;
}

internal double get_val_(nl_matrix* m, uint64_t i) {
    return *(m->data + i * sizeof(double));
}

internal void set_val_(nl_matrix* m, uint64_t i, double v) {
    *(m->data + i * sizeof(double)) = v;
}

internal l_obj* nl_matrix_box(nl_matrix* m) {
    return lean_alloc_external(g_nl_matrix_external_class, m);
}

internal nl_matrix* nl_matrix_unbox(l_obj* o) {
    return (nl_matrix*) lean_get_external_data(o);
}

// API

external l_res nl_initialize() {
    g_nl_matrix_external_class = lean_register_external_class(
        nl_matrix_finalizer,
        noop_foreach
    );
    return lean_io_result_mk_ok(lean_box(0));
}

external l_res nl_matrix_new(uint64_t n_rows, uint64_t n_cols, double v) {
    if (n_rows == 0) {
        return make_error("invalid number of columns");
    }
    if (n_cols == 0) {
        return make_error("invalid number of rows");
    }
    nl_matrix* m = nl_matrix_alloc(n_rows, n_cols);
    if (!m) {
        return make_error("insufficient memory");
    }
    for (uint64_t i = 0; i < m->length; i++) {
        set_val_(m, i, v);
    }
    return lean_io_result_mk_ok(nl_matrix_box(m));
}

external l_res nl_matrix_id(uint64_t n) {
    if (n == 0) {
        return make_error("invalid dimension");
    }
    nl_matrix* m = nl_matrix_alloc(n, n);
    if (!m) {
        return make_error("insufficient memory");
    }
    for (uint64_t i = 0; i < n; i++) {
        for (uint64_t j = 0; j < n; j++) {
            set_val(m, i, j, i == j ? 1.0 : 0.0);
        }
    }
    return lean_io_result_mk_ok(nl_matrix_box(m));
}

external l_res nl_matrix_n_rows(l_arg _m) {
    return lean_box_uint64(nl_matrix_unbox(_m)->n_rows);
}

external l_res nl_matrix_n_cols(l_arg _m) {
    return lean_box_uint64(nl_matrix_unbox(_m)->n_cols);
}

external l_res nl_matrix_get(l_arg _m, uint64_t i, uint64_t j) {
    nl_matrix* m = nl_matrix_unbox(_m);
    if (i >= m->n_rows) {
        return make_error("invalid row index");
    }
    if (j >= m->n_cols) {
        return make_error("invalid column index");
    }
    return lean_io_result_mk_ok(lean_box_float(get_val(m, i, j)));
}

external l_res nl_matrix_plus_float(l_arg _m, double f) {
    nl_matrix* m = nl_matrix_copy(nl_matrix_unbox(_m));
    if (!m) {
        return make_error("insufficient memory");
    }
    for (uint64_t i = 0; i < m->length; i++) {
        set_val_(m, i, f + get_val_(m, i));
    }
    return lean_io_result_mk_ok(nl_matrix_box(m));
}

external l_res nl_matrix_times_float(l_arg _m, double f) {
    nl_matrix* m = nl_matrix_copy(nl_matrix_unbox(_m));
    if (!m) {
        return make_error("insufficient memory");
    }
    for (uint64_t i = 0; i < m->length; i++) {
        set_val_(m, i, f * get_val_(m, i));
    }
    return lean_io_result_mk_ok(nl_matrix_box(m));
}