#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

bool g_solved = false;

int gcd(int a, int b) {
	while (b) {
		int t = b;
		b = a % b;
		a = t;
	}
	return a;
}

typedef struct NumDen {
	int num;
	int den;
} NumDen;

typedef struct SimpleExpr {
	NumDen a;
	NumDen b;
	NumDen res;
	char op;
} SimpleExpr;

typedef void (*op_fn)(NumDen* nd1, NumDen* nd2, NumDen* res);


static bool is_commutative(char op) {
	return op == '+' || op == '*';
}

const char* opstr[4] = {
	" + ",
	" * ",
	" - ",
	" / ",
};

void nd_set(NumDen* nd, int n, int d) {
	nd->num = n;
	nd->den = d;
}

void nd_times(NumDen* nd1, NumDen* nd2, NumDen* res) {
	res->num = nd1->num * nd2->num;
	res->den = nd1->den * nd2->den;
}

void nd_divby(NumDen* nd1, NumDen* nd2, NumDen* res) {
	int n = nd1->num * nd2->den;
	int d = nd1->den * nd2->num;
	res->num = n;
	res->den = d;
}

void nd_plus(NumDen* nd1, NumDen* nd2, NumDen* res) {
	res->num = nd1->num * nd2->den + nd2->num * nd1->den;
	res->den = nd1->den * nd2->den;
}

void nd_minus(NumDen* nd1, NumDen* nd2, NumDen* res) {
	res->num = nd1->num * nd2->den - nd2->num * nd1->den;
	res->den = nd1->den * nd2->den;
}

bool nd_gt(NumDen* nd1, NumDen* nd2) { // true if nd1 > nd2
	// num/den = nd1 - nd2
	int num = nd1->num * nd2->den - nd2->num * nd1->den;
	int den = nd1->den * nd2->den;
	// num / den > 0 ? --> num * den > 0
	return (num * den > 0);
}

void nd_print(NumDen* nd) {
	int n = nd->num;
	int d = nd->den;
	if (d < 0) {
		n = -n;
		d = -d;
	}
	if (n < 0) {
		printf("-");
		n = -n;
	}
	if (n % d == 0) {
		printf("%d", n);
	}
	else {
		int g = gcd(n, d);
		n /= g;
		d /= g;
		printf("%d/%d", n, d);
	}
}

bool check_answer(NumDen* res, int target) {
	return res->den != 0 && res->num == target * res->den;
}

void print_expr(SimpleExpr* e) {
	nd_print(&e->a);
	printf(" %c ", e->op);
	nd_print(&e->b);
	printf(" = ");
	nd_print(&e->res);
}

void print_solution(SimpleExpr* expr_stack, size_t depth) {
	for (size_t ii = 0; ii < depth; ++ii) {
		print_expr(expr_stack + ii);
		printf("\n");
	}
}

void print_expr_stack_debug(SimpleExpr* expr_stack, size_t depth) {
	for (size_t ii = 0; ii < depth; ++ii) {
		print_expr(expr_stack + ii);
		printf("; ");
	}
}

// following array is filled with function pointers
char ops[] = "+-*/";
op_fn g_ops[] = {nd_plus, nd_minus, nd_times, nd_divby};


void solve(int target, NumDen* nrs, size_t nrs_len, SimpleExpr* expr_stack, size_t depth, size_t max_depth) {
	/*
	printf("solve(%d, [", target);
	for (int ii = 0; ii < nrs_len; ++ii) {
		nd_print(nrs + ii);
		if (ii < nrs_len - 1)
			printf(",");
	}
	printf("], %zu, ...\n", nrs_len);
	printf("Depth: %zu\n", depth);
	printf("expr: ");
	print_expr_stack_debug(expr_stack, depth);
	printf("\n");
	//printf("Items on nrs stack: %lu\n", ((void*)(nrs + nrs_len) - dbg_ptr) / sizeof(NumDen));
	*/

	NumDen res;
	// base cases
	if (depth > max_depth)
		return;
	for (size_t ii = 0; ii < nrs_len; ++ii) {
		if (check_answer(nrs + ii, target)) {
			print_solution(expr_stack, depth);
			printf("\n");
			g_solved = true;
			return;
		}
	}
	if (nrs_len < 2) return;

	// brute force all pairs + all ops
	for (int ii = 0; ii < nrs_len; ++ii) {
		NumDen* nd1 = nrs + ii;
		for (int jj = 0; jj < nrs_len; ++jj) {
			if (jj == ii) continue;
			NumDen* nd2 = nrs + jj;
			bool gt = nd_gt(nd1, nd2);
			for (int oo = 0; oo < 4; ++oo) {
				if (is_commutative(ops[oo]) && gt) continue;
				g_ops[oo](nd1, nd2, &res);

				if (res.den == 0) continue;

				SimpleExpr expr = {
					.a = *nd1,
					.b = *nd2,
					.op = ops[oo],
					.res = res
				};
				*(expr_stack + depth) = expr;

				// stack result, and remaining nrs
				NumDen* stack = nrs + nrs_len;
				*stack = res;
				stack++;
				for (int kk = 0; kk < nrs_len; ++kk) {
					if (kk != ii && kk != jj) {
						*stack = *(nrs + kk);
						++stack;
					}
				}
				solve(target, nrs + nrs_len, nrs_len - 1, expr_stack, depth + 1, max_depth);
			}
		}
	}
}

void usage(const char* progname) {
	printf("Usage: %s target nr1 nr2 nr3 ... \n", progname);
}


int main(int argc, char* argv[]) {
	/*
	int input[] = {5, 7, 9, 9, 10, 25};
	int input_count = 6;
	int target = 950;
	*/
	int input[10];
	int input_count = 0;
	int target;

	if (argc < 4) {
		usage(argv[0]);
		return 0;
	}

	target = atoi(argv[1]);
	for (int ii = 2; ii < argc; ++ii) {
		input[input_count++] = atoi(argv[ii]);
	}

	// workspace for number lists and expressions
	size_t ndstack_size = input_count * (input_count + 1) / 2; // first it we store 6 nrs, then 5, then 4 etc.
	NumDen* nd_stack = malloc(ndstack_size * sizeof(NumDen));
	SimpleExpr* expr_stack = malloc(input_count * sizeof(SimpleExpr));

	// init workspace
	for (size_t ii = 0; ii < input_count; ++ii) {
		nd_stack[ii].num = input[ii];
		nd_stack[ii].den = 1;
	}
	for (size_t max_depth = 2; max_depth < input_count - 1; ++max_depth)
		solve(target, nd_stack, input_count, expr_stack, 0, max_depth);

	free(nd_stack);
	free(expr_stack);

	return 0;
}

