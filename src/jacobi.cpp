#include "jacobi.h"

// the Jacobi kernel
// u_cur[i][j] = w1 * (u_prev[i-1][j] + u_prev[i+1][j])
// 				+ w2 * (u_prev[i][j-1] + u_prev[i][j+1])
//				+ w3 * u_prev[i][j] + c[i][j]

void jacobi(
	const d_type *w1,
	const d_type *w2,
	const d_type *w3,
	const d_type u_prev[N][N],
	const d_type c[N][N],
	d_type u_cur[N][N]
) {

#pragma HLS interface m_axi port=w1
#pragma HLS interface m_axi port=w2
#pragma HLS interface m_axi port=w3
#pragma HLS interface m_axi port=u_prev bundle=gmem0
#pragma HLS interface m_axi port=u_cur bundle=gmem0
#pragma HLS interface m_axi port=c bundle=gmem1

	// Load weights to local variables for reuse
	d_type w1_val, w2_val, w3_val;
	w1_val = *w1;
	w2_val = *w2;
	w3_val = *w3;

	// Line buffers for storing three consecutive rows
	// Used for data reuse in vertical direction
	static d_type line_buffer_prev[N];
	static d_type line_buffer_curr[N];
	static d_type line_buffer_next[N];

#pragma HLS array_partition variable=line_buffer_prev cyclic factor=1
#pragma HLS array_partition variable=line_buffer_curr cyclic factor=1
#pragma HLS array_partition variable=line_buffer_next cyclic factor=1

	// Initialize line buffers: load first two rows
	init_loop: for (int j = 0; j < N; j++) {
#pragma HLS pipeline II=1
		line_buffer_prev[j] = 0;  // Boundary condition: row -1 = 0
		line_buffer_curr[j] = u_prev[0][j];
		if (N > 1) {
			line_buffer_next[j] = u_prev[1][j];
		} else {
			line_buffer_next[j] = 0;
		}
	}

	// Main computation loop: iterate over rows
	row_loop: for (int i = 0; i < N; i++) {
#pragma HLS pipeline off

		// Shift registers for horizontal data reuse (left, center, right)
		d_type shift_reg_left = 0;  // Boundary: column -1 = 0
		d_type shift_reg_center = line_buffer_curr[0];
		d_type shift_reg_right;

		// Initialize right shift register
		if (N > 1) {
			shift_reg_right = line_buffer_curr[1];
		} else {
			shift_reg_right = 0;
		}

		// Process each column in current row
		col_loop: for (int j = 0; j < N; j++) {
#pragma HLS pipeline II=1

			// Get vertical neighbors from line buffers
			d_type up = (i > 0) ? line_buffer_prev[j] : 0;
			d_type down = (i < N - 1) ? line_buffer_next[j] : 0;

			// Get horizontal neighbors from shift registers
			d_type left = shift_reg_left;
			d_type center = shift_reg_center;
			d_type right = (j < N - 1) ? shift_reg_right : 0;

			// Compute Jacobi update
			d_type result = w1_val * (up + down) 
			              + w2_val * (left + right) 
			              + w3_val * center 
			              + c[i][j];

			// Write result
			u_cur[i][j] = result;

			// Update shift registers for next iteration
			shift_reg_left = shift_reg_center;
			shift_reg_center = shift_reg_right;
			if (j < N - 2) {
				shift_reg_right = line_buffer_curr[j + 2];
			} else if (j == N - 2) {
				shift_reg_right = 0;  // Boundary: column N = 0
			}
		}

		// Update line buffers for next row iteration (before last row)
		if (i < N - 1) {
			update_line_buffers: for (int j = 0; j < N; j++) {
#pragma HLS pipeline II=1
				line_buffer_prev[j] = line_buffer_curr[j];
				line_buffer_curr[j] = line_buffer_next[j];
				if (i < N - 2) {
					line_buffer_next[j] = u_prev[i + 2][j];
				} else {
					line_buffer_next[j] = 0;  // Boundary: row N = 0
				}
			}
		}
	}
}
