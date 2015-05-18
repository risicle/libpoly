/*
 * value.c
 *
 *  Created on: Apr 3, 2014
 *      Author: dejan
 */


#include <interval.h>
#include <algebraic_number.h>
#include <upolynomial.h>

#include "number/value.h"
#include "number/integer.h"
#include "number/rational.h"
#include "number/dyadic_rational.h"

void lp_value_construct(lp_value_t* v, lp_value_type_t type, const void* data) {
  v->type = type;
  switch(type) {
  case LP_VALUE_PLUS_INFINITY:
  case LP_VALUE_MINUS_INFINITY:
  case LP_VALUE_NONE:
    break;
  case LP_VALUE_INTEGER:
    integer_construct_copy(lp_Z, &v->value.z, data);
    break;
  case LP_VALUE_RATIONAL:
    rational_construct_copy(&v->value.q, data);
    break;
  case LP_VALUE_DYADIC_RATIONAL:
    dyadic_rational_construct_copy(&v->value.dy_q, data);
    break;
  case LP_VALUE_ALGEBRAIC:
    lp_algebraic_number_construct_copy(&v->value.a, data);
    break;
  }
}

void lp_value_construct_zero(lp_value_t* v) {
  v->type = LP_VALUE_INTEGER;
  integer_construct(&v->value.z);
}

void lp_value_assign_raw(lp_value_t* v, lp_value_type_t type, const void* data) {
  lp_value_destruct(v);
  lp_value_construct(v, type, data);
}

void lp_value_assign(lp_value_t* v, const lp_value_t* from) {
  if (v != from) {
    lp_value_destruct(v);
    lp_value_construct_copy(v, from);
  }
}

lp_value_t* lp_value_new(lp_value_type_t type, const void* data) {
  lp_value_t* result = malloc(sizeof(lp_value_t));
  lp_value_construct(result, type, data);
  return result;
}

lp_value_t* lp_value_new_copy(const lp_value_t* from) {
  lp_value_t* result = malloc(sizeof(lp_value_t));
  lp_value_construct_copy(result, from);
  return result;
}

void lp_value_construct_copy(lp_value_t* v, const lp_value_t* from) {
  switch(from->type) {
  case LP_VALUE_NONE:
  case LP_VALUE_PLUS_INFINITY:
  case LP_VALUE_MINUS_INFINITY:
    lp_value_construct(v, from->type, 0);
    break;
  case LP_VALUE_INTEGER:
    lp_value_construct(v, LP_VALUE_INTEGER, &from->value.z);
    break;
  case LP_VALUE_RATIONAL:
    lp_value_construct(v, LP_VALUE_RATIONAL, &from->value.q);
    break;
  case LP_VALUE_DYADIC_RATIONAL:
    lp_value_construct(v, LP_VALUE_DYADIC_RATIONAL, &from->value.dy_q);
    break;
  case LP_VALUE_ALGEBRAIC:
    lp_value_construct(v, LP_VALUE_ALGEBRAIC, &from->value.a);
    break;
  }
}

void lp_value_destruct(lp_value_t* v) {
  switch(v->type) {
  case LP_VALUE_NONE:
  case LP_VALUE_PLUS_INFINITY:
  case LP_VALUE_MINUS_INFINITY:
    break;
  case LP_VALUE_INTEGER:
    integer_destruct(&v->value.z);
    break;
  case LP_VALUE_RATIONAL:
    rational_destruct(&v->value.q);
    break;
  case LP_VALUE_DYADIC_RATIONAL:
    dyadic_rational_destruct(&v->value.dy_q);
    break;
  case LP_VALUE_ALGEBRAIC:
    lp_algebraic_number_destruct(&v->value.a);
    break;
  }
}

void lp_value_delete(lp_value_t* v) {
  lp_value_destruct(v);
  free(v);
}

// 1/2^20
#define LP_VALUE_APPROX_MIN_MAGNITUDE -20

void lp_value_approx(const lp_value_t* v, lp_interval_t* out) {
  int size;

  lp_interval_t approx;

  switch (v->type) {
  case LP_VALUE_NONE:
  case LP_VALUE_PLUS_INFINITY:
  case LP_VALUE_MINUS_INFINITY:
    assert(0);
    break;
  case LP_VALUE_INTEGER: {
    lp_rational_t point;
    rational_construct_from_integer(&point, &v->value.z);
    lp_interval_construct_point(&approx, &point);
    break;
  }
  case LP_VALUE_RATIONAL:
    lp_interval_construct_point(&approx, &v->value.q);
    break;
  case LP_VALUE_DYADIC_RATIONAL:
    lp_interval_construct_from_dyadic(&approx, &v->value.dy_q, 0, &v->value.dy_q, 0);
    break;
  case LP_VALUE_ALGEBRAIC: {
    // Make sure we're below the given size
    size = lp_dyadic_interval_size(&v->value.a.I);
    while (size > LP_VALUE_APPROX_MIN_MAGNITUDE) {
      size --;
      lp_algebraic_number_refine_const(&v->value.a);
    }
    lp_interval_construct_from_dyadic_interval(&approx, &v->value.a.I);
    break;
  }
  }

  lp_interval_swap(&approx, out);
  lp_interval_destruct(&approx);
}

int lp_value_print(const lp_value_t* v, FILE* out) {
  int ret = 0;
  switch (v->type) {
  case LP_VALUE_NONE:
    ret += fprintf(out, "<null>");
    break;
  case LP_VALUE_PLUS_INFINITY:
    ret += fprintf(out, "+inf");
    break;
  case LP_VALUE_MINUS_INFINITY:
    ret += fprintf(out, "-inf");
    break;
  case LP_VALUE_INTEGER:
    ret += integer_print(&v->value.z, out);
    break;
  case LP_VALUE_RATIONAL:
    ret += rational_print(&v->value.q, out);
    break;
  case LP_VALUE_DYADIC_RATIONAL:
    ret += dyadic_rational_print(&v->value.dy_q, out);
    break;
  case LP_VALUE_ALGEBRAIC:
    ret += lp_algebraic_number_print(&v->value.a, out);
    break;
  }
  return ret;
}

int lp_value_cmp(const lp_value_t* v1, const lp_value_t* v2) {

  if (v1->type == v2->type) {
    lp_value_type_t type = v1->type;
    switch (type) {
    case LP_VALUE_NONE:
    case LP_VALUE_PLUS_INFINITY:
    case LP_VALUE_MINUS_INFINITY:
      return 0;
    case LP_VALUE_INTEGER:
      return lp_integer_cmp(lp_Z, &v1->value.z, &v2->value.z);
    case LP_VALUE_RATIONAL:
      return rational_cmp(&v1->value.q, &v2->value.q);
    case LP_VALUE_DYADIC_RATIONAL:
      return dyadic_rational_cmp(&v1->value.dy_q, &v2->value.dy_q);
    case LP_VALUE_ALGEBRAIC:
      return lp_algebraic_number_cmp(&v1->value.a, &v2->value.a);
    }
  }

  // Different types

  if (v1->type == LP_VALUE_MINUS_INFINITY) {
    return -1;
  }
  if (v2->type == LP_VALUE_MINUS_INFINITY) {
    return 1;
  }
  if (v1->type == LP_VALUE_PLUS_INFINITY) {
    return 1;
  }
  if (v2->type == LP_VALUE_PLUS_INFINITY) {
    return -1;
  }

  // Make sure that the first one is bigger in the order int < dy_rat < rat < algebraic
  if (v1->type < v2->type) {
    return -lp_value_cmp(v1, v2);
  }

  switch (v1->type) {
  case LP_VALUE_DYADIC_RATIONAL:
    switch (v2->type) {
    case LP_VALUE_INTEGER:
      return dyadic_rational_cmp_integer(&v1->value.dy_q, &v2->value.z);
    default:
      assert(0);
    }
    break;
  case LP_VALUE_RATIONAL:
    switch (v2->type) {
    case LP_VALUE_INTEGER:
      return rational_cmp_integer(&v1->value.q, &v2->value.z);
    case LP_VALUE_DYADIC_RATIONAL:
      return rational_cmp_dyadic_rational(&v1->value.q, &v1->value.dy_q);
    default:
      assert(0);
    }
    break;
  case LP_VALUE_ALGEBRAIC:
    switch (v2->type) {
    case LP_VALUE_INTEGER:
      return lp_algebraic_number_cmp_integer(&v1->value.a, &v2->value.z);
    case LP_VALUE_DYADIC_RATIONAL:
      return lp_algebraic_number_cmp_dyadic_rational(&v1->value.a, &v2->value.dy_q);
    case LP_VALUE_RATIONAL:
      return lp_algebraic_number_cmp_rational(&v1->value.a, &v2->value.q);
    default:
      assert(0);
    }
    break;
  default:
    assert(0);
  }

  assert(0);

  return v1 == v2;
}

int lp_value_cmp_void(const void* v1, const void* v2) {
  assert(0);
  return v1 == v2;
}

int lp_value_is_rational(const lp_value_t* v) {
  switch (v->type) {
  case LP_VALUE_INTEGER:
  case LP_VALUE_DYADIC_RATIONAL:
  case LP_VALUE_RATIONAL:
    return 1;
    break;
  case LP_VALUE_ALGEBRAIC:
    if (lp_dyadic_interval_is_point(&v->value.a.I)) {
      // If a point, we're (dyadic) rational
      return 1;
    } else if (lp_upolynomial_degree(v->value.a.f) == 1) {
      // If degree 1, we're directly rational
      return 1;
    } else {
      return 0;
    }
    break;
  default:
    return 0;
  }
}

void lp_value_get_num(const lp_value_t* v, lp_integer_t* num) {
  assert(lp_value_is_rational(v));
  switch (v->type) {
  case LP_VALUE_INTEGER:
    integer_assign(lp_Z, num, &v->value.z);
    break;
  case LP_VALUE_DYADIC_RATIONAL:
    dyadic_rational_get_num(&v->value.dy_q, num);
    break;
  case LP_VALUE_RATIONAL:
    rational_get_num(&v->value.q, num);
    break;
  case LP_VALUE_ALGEBRAIC:
    if (lp_dyadic_interval_is_point(&v->value.a.I)) {
      // It's a point value, so we just get it
      dyadic_rational_get_num(lp_dyadic_interval_get_point(&v->value.a.I), num);
    } else {
      const lp_upolynomial_t* v_poly = v->value.a.f;
      if (lp_upolynomial_degree(v_poly) == 1) {
        // p = ax + b = 0 => x = -b/a
        lp_rational_t value;
        rational_construct_from_div(&value,
            /* b */ lp_upolynomial_const_term(v_poly),
            /* a */ lp_upolynomial_lead_coeff(v_poly)
            );
        rational_neg(&value, &value);
        rational_get_num(&value, num);
        rational_destruct(&value);
      } else {
        assert(0);
      }
    }
    break;
  default:
    assert(0);
  }
}

void lp_value_get_den(const lp_value_t* v, lp_integer_t* den) {
  assert(lp_value_is_rational(v));
  switch (v->type) {
  case LP_VALUE_INTEGER:
    integer_assign_int(lp_Z, den, 1);
    break;
  case LP_VALUE_DYADIC_RATIONAL:
    dyadic_rational_get_den(&v->value.dy_q, den);
    break;
  case LP_VALUE_RATIONAL:
    rational_get_den(&v->value.q, den);
    break;
  case LP_VALUE_ALGEBRAIC:
    if (lp_dyadic_interval_is_point(&v->value.a.I)) {
      // It's a point value, so we just get it
      dyadic_rational_get_den(lp_dyadic_interval_get_point(&v->value.a.I), den);
    } else {
      const lp_upolynomial_t* v_poly = v->value.a.f;
      if (lp_upolynomial_degree(v_poly) == 1) {
        // p = ax + b = 0 => x = -b/a
        lp_rational_t value;
        rational_construct_from_div(&value,
            /* b */ lp_upolynomial_const_term(v_poly),
            /* a */ lp_upolynomial_lead_coeff(v_poly)
            );
        rational_neg(&value, &value);
        rational_get_den(&value, den);
        rational_destruct(&value);
      } else {
        assert(0);
      }
    }
    break;
  default:
    assert(0);
  }
}

char* lp_value_to_string(const lp_value_t* v) {
  char* str = 0;
  size_t size = 0;
  FILE* f = open_memstream(&str, &size);
  lp_value_print(v, f);
  fclose(f);
  return str;
}
