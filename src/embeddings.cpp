// This tool generates word embeddings from the Hellinger PCA.
//
// Copyright (c) 2015 Idiap Research Institute, http://www.idiap.ch/
// Written by Rémi Lebret <remi@lebret.ch>
//
// This file is part of HPCA.
//
// HPCA is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 3 as
// published by the Free Software Foundation.
//
// HPCA is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with HPCA. If not, see <http://www.gnu.org/licenses/>.

// C++ header
#include <stdexcept>

// include utility headers
#include "util/thread.h"
#include "util/util.h"
#include "util/convert.h"
#include "util/file.h"
#include "util/data.h"
#include "util/constants.h"

// include redsvd headers
#include "redsvd/util.h"
#include "redsvd/redsvd.h"
#include "redsvd/redsvdFile.h"

// include i/o headers
#include "io/eigen.h"

// define global variables
int verbose = true; // true or false
int norm = false; // true or false
int num_threads = 8;
int dim = 100;
float eig=0.0;
char *c_input_dir_name, *c_input_file_U_name, *c_input_file_S_name;
char *c_output_file_name;

int run() {
    // read U matrix from file
    if (verbose) fprintf(stderr, "reading singular vectors in %s\n", c_input_file_U_name);
    Eigen::MatrixXf U;
    read_eigen_truncated_matrix(c_input_file_U_name, U, dim);

    // read S matrix from file
    if (verbose) fprintf(stderr, "reading singular values in %s\n", c_input_file_S_name);
    Eigen::VectorXf S;
    read_eigen_vector(c_input_file_S_name, S, dim, eig);

    Eigen::MatrixXf embeddings = U * S.asDiagonal();
    if (norm) embeddings.rowwise().normalize();

    // write out embeddings
    const char *c_output_name = get_full_path(c_input_dir_name, c_output_file_name);
    if (verbose) fprintf(stderr, "writing word embeddings in %s\n", c_output_name);
    write_eigen_matrix(c_output_name, embeddings);
    free((char*)c_output_name);

    return 0;
}

int main(int argc, char **argv) {
    int i;
    c_input_dir_name = (char*)malloc(sizeof(char) * MAX_PATH_NAME);
    c_output_file_name = (char*)malloc(sizeof(char) * MAX_FILE_NAME);

    if (argc == 1) {
        printf("HPCA: Hellinger PCA for Word Embeddings, embeddings computation\n");
        printf("Author: Remi Lebret (remi@lebret.ch)\n\n");
        printf("Usage options:\n");
        printf("\t-verbose <int>\n");
        printf("\t\tSet verbosity: 0=off or 1=on (default)\n");
        printf("\t-input-dir <dir>\n");
        printf("\t\tDirectory where to find files from the SVD\n");
        printf("\t-output-name <string>\n");
        printf("\t\tFilename for embeddings file which will be placed in <input-dir> (default is words.txt)\n");
        printf("\t-eig <float>\n");
        printf("\t\tEigenvalue weighting (0.0, 0.5 or 1.0); default is 0.0\n");
        printf("\t-dim <int>\n");
        printf("\t\tWord vector dimension; default 100\n");
        printf("\t-norm <int>\n");
        printf("\t\tAre vectors normalized to unit length? 0=off or 1=on (default is 0)\n");
        printf("\t-threads <int>\n");
        printf("\t\tNumber of threads; default 8\n");
        printf("\nExample usage:\n");
        printf("./embeddings -input-dir path_to_svd_files -output-name words.txt -eig 0.0 -dim 100 -norm 0\n\n");
        return 0;
    }

    if ((i = find_arg((char *)"-verbose", argc, argv)) > 0) verbose = atoi(argv[i + 1]);
    if ((i = find_arg((char *)"-norm", argc, argv)) > 0) norm = atoi(argv[i + 1]);
    if ((i = find_arg((char *)"-dim", argc, argv)) > 0) dim = atoi(argv[i + 1]);
    if ((i = find_arg((char *)"-eig", argc, argv)) > 0) eig = atof(argv[i + 1]);
    if ((i = find_arg((char *)"-input-dir", argc, argv)) > 0) strcpy(c_input_dir_name, argv[i + 1]);
    if ((i = find_arg((char *)"-output-name", argc, argv)) > 0) strcpy(c_output_file_name, argv[i + 1]);
    else strcpy(c_output_file_name, "words.txt");
    if ((i = find_arg((char *)"-threads", argc, argv)) > 0) num_threads = atoi(argv[i + 1]);

    if (verbose){
        fprintf(stderr, "HPCA: Hellinger PCA for Word Embeddings\n");
        fprintf(stderr, "Author: Remi Lebret (remi@lebret.ch)\n");
        fprintf(stderr, "---------------------------------------\n");
        fprintf(stderr, "embeddings computation\n" );
        fprintf(stderr, "---------------------------------------\n\n");
    }
    /* set the optimal number of threads */
    num_threads = MultiThread::optimal_nb_thread(num_threads, 1, num_threads);
    if (verbose) fprintf(stderr, "number of pthreads = %d\n", num_threads);
    // set threads
    Eigen::setNbThreads(num_threads);

    /* check whether input directory exists */
    is_directory(c_input_dir_name);

    /* check parameters */
    if ( (eig<0) || (eig>1) ){
        throw std::runtime_error("-eig must be a value between 0 and 1 !!");
    }
    if ( dim<=0 ){
        throw std::runtime_error("-dim must be a positive integer!!");
    }

    /* create output filenames */
    c_input_file_U_name = get_full_path(c_input_dir_name, "svd.U");
    c_input_file_S_name = get_full_path(c_input_dir_name, "svd.S");

    /* check whether files exist */
    is_file( c_input_file_U_name );
    is_file( c_input_file_S_name );

    /* compute embeddings */
    run();

    /* release memory */
    free(c_input_dir_name);
    free(c_output_file_name);
    free(c_input_file_U_name);
    free(c_input_file_S_name);

    if (verbose){
        fprintf(stderr, "\ndone\n");
        fprintf(stderr, "---------------------------------------\n");
    }
    return 0;
}
