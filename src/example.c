/*	example.c
 *	This is a simple example to show you how to use the SSW C library.
 *	To run this example:
 *	1) gcc -Wall -lz ssw.c example.c
 *	2) ./a.out
 *	Created by Mengyao Zhao on 07/31/12.
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include "ssw.h"

//	Print the BLAST like output.
void ssw_write (s_align* a,
			char* ref_seq,
			char* read_seq,
			int8_t* table) {

	fprintf(stdout, "optimal_alignment_score: %d\tsub-optimal_alignment_score: %d\t", a->score1, a->score2);
	if (a->ref_begin1 + 1) fprintf(stdout, "target_begin: %d\t", a->ref_begin1 + 1);
	fprintf(stdout, "target_end: %d\t", a->ref_end1 + 1);
	if (a->read_begin1 + 1) fprintf(stdout, "query_begin: %d\t", a->read_begin1 + 1);
	fprintf(stdout, "query_end: %d\n\n", a->read_end1 + 1);
	if (a->cigar) {
		int32_t i, c = 0, left = 0, e = 0, qb = a->ref_begin1, pb = a->read_begin1;
		while (e < a->cigarLen || left > 0) {
			int32_t count = 0;
			int32_t q = qb;
			int32_t p = pb;
			fprintf(stdout, "Target: %8d    ", q + 1);
			for (c = e; c < a->cigarLen; ++c) {
				int32_t letter = 0xf&*(a->cigar + c);
				int32_t length = (0xfffffff0&*(a->cigar + c))>>4;
				int32_t l = (count == 0 && left > 0) ? left: length;
				for (i = 0; i < l; ++i) {
					if (letter == 1) fprintf(stdout, "-");
					else {
						fprintf(stdout, "%c", *(ref_seq + q));
						++ q;
					}
					++ count;
					if (count == 60) goto step2;
				}
			}
step2:
			fprintf(stdout, "    %d\n                    ", q);
			q = qb;
			count = 0;
			for (c = e; c < a->cigarLen; ++c) {
				int32_t letter = 0xf&*(a->cigar + c);
				int32_t length = (0xfffffff0&*(a->cigar + c))>>4;
				int32_t l = (count == 0 && left > 0) ? left: length;
				for (i = 0; i < l; ++i){
					if (letter == 0) {
						if (table[(int)*(ref_seq + q)] == table[(int)*(read_seq + p)])fprintf(stdout, "|");
						else fprintf(stdout, "*");
						++q;
						++p;
					} else {
						fprintf(stdout, "*");
						if (letter == 1) ++p;
						else ++q;
					}
					++ count;
					if (count == 60) {
						qb = q;
						goto step3;
					}
				}
			}
step3:
			p = pb;
			fprintf(stdout, "\nQuery:  %8d    ", p + 1);
			count = 0;
			for (c = e; c < a->cigarLen; ++c) {
				int32_t letter = 0xf&*(a->cigar + c);
				int32_t length = (0xfffffff0&*(a->cigar + c))>>4;
				int32_t l = (count == 0 && left > 0) ? left: length;
				for (i = 0; i < l; ++i) {
					if (letter == 2) fprintf(stdout, "-");
					else {
						fprintf(stdout, "%c", *(read_seq + p));
						++p;
					}
					++ count;
					if (count == 60) {
						pb = p;
						left = l - i - 1;
						e = (left == 0) ? (c + 1) : c;
						goto end;
					}
				}
			}
			e = c;
			left = 0;
end:
			fprintf(stdout, "    %d\n\n", p);
		}
	}
}

//	Align a pair of genome sequences.
int main (int argc, char * const argv[]) {
    // default parameters for genome sequence alignment
	int32_t l, m, k, match = 2, mismatch = 2, gap_open = 3, gap_extension = 1;
    // from Mengyao's example about the importance of using all three matrices in traceback.
  // int32_t l, m, k, match = 2, mismatch = 1, gap_open = 2, gap_extension = 1;


	// reference sequence
    /*
	char ref_seq[40] = {'C', 'A', 'G', 'C', 'C', 'T', 'T', 'T', 'C', 'T', 'G', 'A', 'C', 'C', 'C', 'G', 'G', 'A', 'A', 'A', 'T',
						'C', 'A', 'A', 'A', 'A', 'T', 'A', 'G', 'G', 'C', 'A', 'C', 'A', 'A', 'C', 'A', 'A', 'A', '\0'};
	char read_seq[16] = {'C', 'T', 'G', 'A', 'G', 'C', 'C', 'G', 'G', 'T', 'A', 'A', 'A', 'T', 'C', '\0'};	// read sequence
    */

    char *ref_seq_1 = argv[1];
    char *ref_seq_2 = argv[2];
    char *read_seq = argv[3];

	s_profile* profile;
	int8_t* num = (int8_t*)malloc(strlen(read_seq));	// the read sequence represented in numbers
	int8_t* ref_num_1 = (int8_t*)malloc(strlen(ref_seq_1));	// the read sequence represented in numbers
	int8_t* ref_num_2 = (int8_t*)malloc(strlen(ref_seq_2));	// the read sequence represented in numbers
	s_align* result;

	/* This table is used to transform nucleotide letters into numbers. */
	int8_t nt_table[128] = {
		4, 4, 4, 4,  4, 4, 4, 4,  4, 4, 4, 4,  4, 4, 4, 4,
		4, 4, 4, 4,  4, 4, 4, 4,  4, 4, 4, 4,  4, 4, 4, 4,
		4, 4, 4, 4,  4, 4, 4, 4,  4, 4, 4, 4,  4, 4, 4, 4,
		4, 4, 4, 4,  4, 4, 4, 4,  4, 4, 4, 4,  4, 4, 4, 4,
		4, 0, 4, 1,  4, 4, 4, 2,  4, 4, 4, 4,  4, 4, 4, 4,
		4, 4, 4, 4,  3, 0, 4, 4,  4, 4, 4, 4,  4, 4, 4, 4,
		4, 0, 4, 1,  4, 4, 4, 2,  4, 4, 4, 4,  4, 4, 4, 4,
		4, 4, 4, 4,  3, 0, 4, 4,  4, 4, 4, 4,  4, 4, 4, 4
	};

	// initialize scoring matrix for genome sequences
	//  A  C  G  T	N (or other ambiguous code)
	//  2 -2 -2 -2 	0	A
	// -2  2 -2 -2 	0	C
	// -2 -2  2 -2 	0	G
	// -2 -2 -2  2 	0	T
	//	0  0  0  0  0	N (or other ambiguous code)
	int8_t* mat = (int8_t*)calloc(25, sizeof(int8_t));
	for (l = k = 0; l < 4; ++l) {
		for (m = 0; m < 4; ++m) mat[k++] = l == m ? match : - mismatch;	/* weight_match : -weight_mismatch */
		mat[k++] = 0; // ambiguous base: no penalty
	}
	for (m = 0; m < 5; ++m) mat[k++] = 0;

	for (m = 0; m < strlen(read_seq); ++m) num[m] = nt_table[(int)read_seq[m]];
	profile = ssw_init(num, strlen(read_seq), mat, 5, 2);
	for (m = 0; m < strlen(ref_seq_1); ++m) ref_num_1[m] = nt_table[(int)ref_seq_1[m]];
	for (m = 0; m < strlen(ref_seq_2); ++m) ref_num_2[m] = nt_table[(int)ref_seq_2[m]];

	// Only the 8 bit of the flag is setted. ssw_align will always return the best alignment beginning position and cigar.
	//result = ssw_align (profile, ref_num, strlen(ref_seq), gap_open, gap_extension, 1, 0, 0, 15);
	//ssw_write(result, ref_seq, read_seq, nt_table);

	result = ssw_fill (profile, ref_num_1, strlen(ref_seq_1), gap_open, gap_extension, 1, 0, 0, 15, 0, NULL);
    print_score_matrix(ref_seq_1, strlen(ref_seq_1), read_seq, strlen(read_seq), result);
    cigar* path = trace_back_byte (result, result->ref_end1, result->read_end1, ref_seq_1, strlen(ref_seq_1), read_seq, strlen(read_seq), match, mismatch, gap_open, gap_extension);
    print_cigar(path); printf("\n");

	result = ssw_fill (profile, ref_num_2, strlen(ref_seq_2), gap_open, gap_extension, 1, 0, 0, 15, 1, result);
    print_score_matrix(ref_seq_2, strlen(ref_seq_2), read_seq, strlen(read_seq), result);

    /*
    int16_t* t;
    int32_t ti;
    for (t = (int16_t*)&result->pvE, ti = 0; ti < 8; ++ti) fprintf(stdout, "%d\t", *t++);
    fprintf(stdout, "\n");
    */

	free(mat);
	free(ref_num_1);
	free(ref_num_2);
	free(num);
	return(0);
}
