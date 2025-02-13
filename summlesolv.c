#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

bool g_solved = false;

typedef struct SimpleExpr {
	int a;
	int b;
	int res;
	char op;
} SimpleExpr;

static bool is_commutative(char op) {
	return op == '+' || op == '*';
}

void print_expr(SimpleExpr* e) {
	printf("%d %c %d = %d", e->a, e->op, e->b, e->res);
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

char ops[] = "+-*/";

void solve(int target, int* nrs, size_t nrs_len, SimpleExpr* expr_stack, size_t depth, size_t max_depth) {
	int res;
	// base cases
	if (depth > max_depth)
		return;
	for (size_t ii = 0; ii < nrs_len; ++ii) {
		if (nrs[ii] == target) {
			print_solution(expr_stack, depth);
			printf("\n");
			g_solved = true;
			return;
		}
	}
	if (nrs_len < 2) return;

	// brute force all pairs + all ops
	for (int ii = 0; ii < nrs_len; ++ii) {
		int n1 = nrs[ii];
		for (int jj = 0; jj < nrs_len; ++jj) {
			if (jj == ii) continue;
			int n2 = nrs[jj];
			for (int oo = 0; oo < 4; ++oo) {
				char op = ops[oo];
				if (is_commutative(op) && n1 > n2) continue;
				if (op == '/' && n2 == 0) continue;
				if (op == '/' && n1 % n2 != 0) continue;

				switch (op) {
					case '+':
						res = n1 + n2;
						break;
					case '-':
						res = n1 - n2;
						break;
					case '*':
						res = n1 * n2;
						break;
					case '/':
						res = n1 / n2;
						break;
				}

				SimpleExpr* e = expr_stack + depth;
				e->a = n1;
				e->op = op;
				e->b = n2;
				e->res = res;

				// stack result, and remaining nrs
				int* stack = nrs + nrs_len;
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
	int* num_stack = malloc(ndstack_size * sizeof(int));
	SimpleExpr* expr_stack = malloc(input_count * sizeof(SimpleExpr));

	// init workspace
	for (size_t ii = 0; ii < input_count; ++ii) {
		num_stack[ii] = input[ii];
	}
	for (size_t max_depth = 2; max_depth < input_count - 1; ++max_depth)
		solve(target, num_stack, input_count, expr_stack, 0, max_depth);

	free(num_stack);
	free(expr_stack);

	return 0;
}

