#ifndef SOSA_H
#define SOSA_H

#include <pthread.h>
#include <signal.h>
#include <time.h>

#include "sos.h"
#include "sosd.h"
#include "sos_types.h"


#define SOSA_DEFAULT_RESULT_ROW_MAX 128
#define SOSA_DEFAULT_RESULT_COL_MAX 24


typedef enum {
    SOSA_OUTPUT_DEFAULT    = 0,          /* CSV w/no header */
    SOSA_OUTPUT_JSON       = (1 << 0),
    SOSA_OUTPUT_W_HEADER   = (1 << 1)   /* Ignored by _JSON */
} SOSA_output_options;


typedef struct {
    SOS_runtime *sos_context;
    int          col_max;
    int          col_count;
    char       **col_names;
    int          row_max;
    int          row_count;
    char      ***data;
} SOSA_results;

/*
 * DEPRECATED: SOSA modules use the standard SOS_runtime now.
 *
typedef struct {
    SOS_runtime *sos_context;
    MPI_Comm     comm;
    int          analytics_color;
    int         *analytics_locales;
    int          world_rank;
    int          world_size;
    int         *world_roles;
    char        *world_hosts;
    int          db_role_count;
    int         *db_role_ranks;
    int          db_target_rank;
} SOSA_runtime;
extern SOSA_runtime SOSA;
 *
 */


/* Required if included by C++ code. */
#ifdef __cplusplus
extern "C" {
#endif

    void SOSA_guid_request(SOS_runtime *sos_context, SOS_uid *uid);
    void SOSA_exec_query(SOS_runtime *sos_context,
        char *sql_string, SOSA_results *result_object);

    void SOSA_results_init(SOS_runtime *sos_context, SOSA_results **results_object_ptraddr);
    void SOSA_results_grow_to(SOSA_results *results, int new_col_max, int new_row_max);
    void SOSA_results_put_name(SOSA_results *results, int col, const char *name);
    void SOSA_results_put(SOSA_results *results, int col, int row, const char *value);
    void SOSA_results_output_to(FILE *file, SOSA_results *results, char *title, int options);
    void SOSA_results_to_buffer(SOS_buffer *buffer, SOSA_results *results);
    void SOSA_results_from_buffer(SOSA_results *results, SOS_buffer *buffer);
    void SOSA_results_wipe(SOSA_results *results_object);
    void SOSA_results_destroy(SOSA_results *results_object);

    void SOSA_send_to_target_db(SOS_buffer *message, SOS_buffer *reply);





#ifdef __cplusplus
}
#endif



#endif
