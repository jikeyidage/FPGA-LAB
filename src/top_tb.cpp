#include "jacobi.h"
#include <iostream>
#include <cmath>

const double EPS = 1e-6;

void compute_gt(const d_type w1, const d_type w2, const d_type w3, const d_type arr[N][N], const d_type c[N][N], d_type gt[N][N]) {
	for(int i = 0; i < N; i++) {
		for(int j = 0; j < N; j++) {
			gt[i][j] = arr[i][j] * w3 + c[i][j];
		}
	}
	for(int i = 0; i < N; i++) {
		for(int j = 1; j < N; j++) {
			gt[i][j] += arr[i][j - 1] * w2;
		}
	}
	for(int i = 0; i < N; i++) {
		for(int j = 0; j < N - 1; j++) {
			gt[i][j] += arr[i][j + 1] * w2;
		}
	}
	for(int i = 1; i < N; i++) {
		for(int j = 0; j < N; j++) {
			gt[i][j] += arr[i - 1][j] * w1;
		}
	}
	for(int i = 0; i < N - 1; i++) {
		for(int j = 0; j < N; j++) {
			gt[i][j] += arr[i + 1][j] * w1;
		}
	}
}


int main() {
	static d_type u0[N][N], c[N][N], u1_gt[N][N], u1_out[N][N];
	d_type w1 = 0.1, w2 = 0.3, w3 = 0.2;

	for(int i = 0; i < N; i++) {
		for(int j = 0; j < N; j++) {
			u0[i][j] = rand() % 100;
			c[i][j] = rand() % 100;
		}
	}

	compute_gt(w1, w2, w3, u0, c, u1_gt);

	jacobi(&w1, &w2, &w3, u0, c, u1_out);
	for(int j = 0; j < N; j++) {
		for(int i = 0; i < N; i++) {
			if(std::fabs(u1_out[i][j] - u1_gt[i][j]) > EPS) {
				std::cout << "Jacobi correctness test failed at (i,j): " << i << " " << j << std::endl;
				return 1;
			}
		}
	}
	std::cout << "Correctness test PASSED" << std::endl;
	return 0;
}
