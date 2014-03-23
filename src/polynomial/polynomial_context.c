/*
 * polynomial_context.c
 *
 *  Created on: Feb 13, 2014
 *      Author: dejan
 */

#include "polynomial_context.h"

#include <malloc.h>
#include <assert.h>

static
void polynomial_context_construct(polynomial_context_t* ctx, int_ring_t* K, variable_db_t* var_db, variable_order_t* var_order) {
  ctx->ref_count = 0;
  ctx->var_db = var_db;
  ctx->K = K;
  ctx->var_db = var_db;
  ctx->var_order = var_order;
}
void polynomial_context_attach(polynomial_context_t* ctx) {
  if (ctx->K) {
    int_ring_ops.attach(ctx->K);
  }
  if (ctx->var_db) {
    variable_db_ops.attach(ctx->var_db);
  }
  if (ctx->var_order) {
    ctx->var_order->ops->attach(ctx->var_order);
  }
  ctx->ref_count ++;
}

void polynomial_context_detach(polynomial_context_t* ctx) {
  if (ctx->K) {
    int_ring_ops.detach(ctx->K);
  }
  if (ctx->var_db) {
    variable_db_ops.detach(ctx->var_db);
  }
  if (ctx->var_order) {
    ctx->var_order->ops->detach(ctx->var_order);
  }
  assert(ctx->ref_count > 0);
  ctx->ref_count --;
  if (ctx->ref_count == 0) {
    free(ctx);
  }
}

polynomial_context_t* polynomial_context_new(int_ring_t* K, variable_db_t* var_db, variable_order_t* var_order) {
  polynomial_context_t* result = malloc(sizeof(polynomial_context_t));
  polynomial_context_construct(result, K, var_db, var_order);
  polynomial_context_attach(result);
  return result;
}


int polynomial_context_equal(const polynomial_context_t* ctx1, const polynomial_context_t* ctx2) {
  if (ctx1 == ctx2) return 1;
  if (ctx1 && ctx2) {
    return int_ring_ops.equal(ctx1->K, ctx2->K) && ctx1->var_order == ctx2->var_order;
  } else {
    return 0;
  }
}

const polynomial_context_ops_t polynomial_context_ops = {
    polynomial_context_new,
    polynomial_context_attach,
    polynomial_context_detach,
    polynomial_context_equal
};