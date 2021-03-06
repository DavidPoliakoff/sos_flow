
/*
 * template.c    (SOSA analytics using MPI example)
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>

#if (SOSD_CLOUD_SYNC > 0)
#include <mpi.h>
#endif

#define USAGE "./sosd_shell -d <initial_delay_seconds>"

#include "sos.h"
#include "sosd.h"
#include "sosa.h"
#include "sos_debug.h"


int main(int argc, char *argv[]) {


    /* Process command-line arguments */
    if ( argc < 3 ) { fprintf(stderr, "%s\n", USAGE); exit(1); }

    int initial_delay_seconds = 0;
    int elem, next_elem = 0;

    for (elem = 1; elem < argc; ) {
        if ((next_elem = elem + 1) == argc) {
            fprintf(stderr, "%s\n", USAGE);
            exit(1);
        }

        if ( strcmp(argv[elem], "-d"  ) == 0) {
            initial_delay_seconds  = atoi(argv[next_elem]);
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
        fprintf(stderr, "ERROR: Could not connect to SOS daemon.\n");
        exit(EXIT_FAILURE);
    }
    srandom(SOS->my_guid);

    sleep(initial_delay_seconds);


    /*
  ****   [ insert your analytics code here ]
     *
     */



    /*
  ****
     */

    return (EXIT_SUCCESS);
}
