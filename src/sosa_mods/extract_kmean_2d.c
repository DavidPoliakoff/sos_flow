
/*
 *  extract_kmean_2d.c   (pull out the x,y pairs plotted by the workers) 
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>

#define USAGE "./sosa_extract_kmean_2d -d <initial_delay_seconds> -o <output_file>"

// NOTE:SOSA_MODULE_COLOR ....... should be unique among active sosa modules.
#define SOSA_MODULE_COLOR   80822

#include "sos.h"
#include "sosa.h"


char SQL_COMMAND[] = ""\
    " SELECT "\
    "   tblPubs.name      AS prog_name, "\
    "   tblPubs.comm_rank AS comm_rank, "\
    "   tblVals.frame     AS frame, "\
    "   tblData.name      AS val_name, "\
    "   tblVals.val       AS value, "\
    " FROM "\
    "   tblVals "\
    "       LEFT JOIN tblData  ON tblVals.guid       = tblData.guid "\
    "       LEFT JOIN tblPubs  ON tblData.pub_guid   = tblPubs.guid "\
    " ; ";

char *file_path;


int main(int argc, char *argv[]) {


    /* Process command-line arguments */
    if ( argc < 5 ) { fprintf(stderr, "%s\n", USAGE); exit(1); }

    int initial_delay_seconds = 0;
    int elem, next_elem = 0;

    file_path = NULL;

    for (elem = 1; elem < argc; ) {
        if ((next_elem = elem + 1) == argc) {
            fprintf(stderr, "%s\n", USAGE);
            exit(1);
        }

        if ( strcmp(argv[elem], "-d"  ) == 0) {
            initial_delay_seconds  = atoi(argv[next_elem]);
        } else if ( strcmp(argv[elem], "-o" ) == 0) {
            file_path = argv[next_elem];
        } else {
            fprintf(stderr, "ERROR: Unknown flag: %s %s\n", argv[elem], argv[next_elem]);
            fprintf(stderr, "%s\n", USAGE);
            exit(1);
        }
        elem = next_elem + 1;
    }

    SOS_runtime *SOS = NULL;
    SOS_init(&argc, &argv, &SOS,
        SOS_ROLE_ANALYTICS, SOS_RECEIVES_NO_FEEDBACK, NULL);
    if (SOS == NULL) {
        fprintf(stderr, "ERROR: Could not initialize SOS.\n");
        exit (EXIT_FAILURE);
    }

    srandom(SOS->my_guid);
    sleep(initial_delay_seconds);


    /*
  ****   [ insert your analytics code here ]
     *
     */



    SOSA_results *results;
    SOSA_results_init(SOS, &results);

    char *query    = "SELECT MAX(val) FROM tblVals;";
    int   count    = 0;

    FILE *fptr = NULL;
    if (file_path == NULL) {
      fptr = fopen("default.csv", "w");
    } else {
      fptr = fopen(file_path, "w");
    }

    SOSA_results_wipe(results);
    SOSA_exec_query(SOS, query, "localhost", atoi(getenv("SOS_CMD_PORT")));
    SOSA_results_output_to(fptr, results, "extract_kmeans_2d", SOSA_OUTPUT_DEFAULT);
    fclose(fptr);
    SOSA_results_destroy(results);
    
    /*
  ****
     */
   SOS_finalize(SOS);


    return (EXIT_SUCCESS);
}
