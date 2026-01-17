#pragma once

// You shall not change the following code
const int N = 1024;
typedef double d_type;

void jacobi(
	const d_type *w1,
	const d_type *w2,
	const d_type *w3,
	const d_type u_prev[N][N],
	const d_type c[N][N],
	d_type u_cur[N][N]
);
