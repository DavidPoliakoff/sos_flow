#include "worker.h"
#include "stdio.h"
#include "stdlib.h"
#include "adios.h"
#include "sos.h"

/* 
 * Worker A is the first application in the workflow.
 * It does some "computation" and communication for N
 * iterations, and it produces "output" that is "input" to worker_b.
 */

int iterations = 1;
bool send_to_b = true;
extern SOS_pub *example_pub;

void validate_input(int argc, char* argv[]) {
    if (argc < 2) {
        my_printf("Usage: %s <num iterations> <send to next worker>\n", argv[0]);
        exit(1);
    }
    /*
    if (commsize < 2) {
        my_printf("%s requires at least 2 processes.\n", argv[0]);
        exit(1);
    }
    */
    iterations = atoi(argv[1]);
    if (argc > 2) {
        int tmp = atoi(argv[2]);
        if (tmp == 0) {
            send_to_b = false;
        }
    }
}

int worker(int argc, char* argv[]) {
    TAU_PROFILE_TIMER(timer, __func__, __FILE__, TAU_USER);
    TAU_PROFILE_START(timer);
    static bool announced = false;
    my_printf("%d of %d In worker A\n", myrank, commsize);

    /* validate input */
    validate_input(argc, argv);

    my_printf("Worker A will execute %d iterations.\n", iterations);

    /* ADIOS: These declarations are required to match the generated
     *        gread_/gwrite_ functions.  (And those functions are
     *        generated by calling 'gpp.py adios_config.xml') ...
     */
    uint64_t  adios_groupsize;
    uint64_t  adios_totalsize;
    uint64_t  adios_handle;
    char      adios_filename[256];
    MPI_Comm  adios_comm;

    /* ADIOS: Can duplicate, split the world, whatever.
     *        This allows you to have P writers to N files.
     *        With no splits, everyone shares 1 file, but
     *        can write lock-free by using different areas.
     */
    //MPI_Comm_dup(MPI_COMM_WORLD, &adios_comm);
    adios_comm = MPI_COMM_WORLD;

    int NX = 10;
    int NY = 1;
    double t[NX];
    double p[NX];

    /* ADIOS: Set up the adios communications and buffers, open the file.
     */
    if (send_to_b) {
        sprintf(adios_filename, "adios_a_to_b.bp");
        adios_init("adios_config.xml", adios_comm);
    }

    int index, i;
    for (index = 0 ; index < iterations ; index++ ) {
        /* Do some exchanges with neighbors */
        do_neighbor_exchange();
        /* "Compute" */
        compute(index);
        /* Write output */
        //my_printf("a");

        for (i = 0; i < NX; i++) {
            t[i] = index*100.0 + myrank*NX + i;
        }

        for (i = 0; i < NY; i++) {
            p[i] = index*1000.0 + myrank*NY + i;
        }

        if (send_to_b) {
            TAU_PROFILE_TIMER(adiostimer, "ADIOS send", __FILE__, TAU_USER);
            TAU_PROFILE_START(adiostimer);
            if (index == 0) {
                adios_open(&adios_handle, "a_to_b", adios_filename, "w", adios_comm);
            } else {
                adios_open(&adios_handle, "a_to_b", adios_filename, "a", adios_comm);
            }
            /* ADIOS: Actually write the data out.
            *        Yes, this is the recommended method, and this way, changes in
            *        configuration with the .XML file will, even in the worst-case
            *        scenario, merely require running 'gpp.py adios_config.xml'
            *        and typing 'make'.
            */
            #include "gwrite_a_to_b.ch"
            /* ADIOS: Close out the file completely and finalize.
            *        If MPI is being used, this must happen before MPI_Finalize().
            */
            adios_close(adios_handle);
            TAU_PROFILE_STOP(adiostimer);
            #if 1
            if (!announced) {
                SOS_val foo;
                foo.i_val = NX;
                SOS_pack(example_pub, "NX", SOS_VAL_TYPE_INT, foo);
                SOS_announce(example_pub);
                SOS_publish(example_pub);
                announced = true;
            }
            #endif
        }
        MPI_Barrier(MPI_COMM_WORLD);
    }
    MPI_Barrier(MPI_COMM_WORLD);

    if (send_to_b) {
        adios_finalize(myrank);
    }
    my_printf("Worker A exting.\n");
    //MPI_Comm_free(&adios_comm);

    TAU_PROFILE_STOP(timer);
    /* exit */
    return 0;
}
