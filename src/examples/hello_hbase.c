/**
 * hello_hbase.c
 *   A sample using libhbase: 同步方式, 测试 api 是否可用
 *
 * Build sample (see: build-hello_hbase.sh):
 *
 * gcc -o hello_hbase hello_hbase.c \
 *   -I../../target/libhbase-1.0-SNAPSHOT/include \
 *   -L../../target/libhbase-1.0-SNAPSHOT/lib/native -lhbase \
 *   -L/usr/local/java/jre/lib/amd64/server -ljvm \
 *   -lpthread \
 *   -lrt
 */
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>

#include <hbase/hbase.h>

#include "wait_done.h"


/*
 * Sample code to illustrate usage of libhbase APIs
 */

#ifdef __cplusplus
extern  "C" {
#endif

#define HTABLE_NAME  "libhbase_test"

#define GET_THREADS    400
#define GET_COUNT_MAX  200000


/* = hbase.zookeeper.ensemble
 *   "zkhost:zkport,zkhost2:zkport,zkhost3:zkport,..."
 */
#define ZK_QUORUM    "localhost:2182"

/* = zookeeper.znode.parent */
#define ZK_ZNODE_ROOT   NULL


static int ensure_hbase_table (hb_connection_t connection, const char *table_name)
{
    int32_t retCode = -1;
    hb_admin_t admin = NULL;

    if ((retCode = hb_admin_create(connection, &admin)) != 0) {
        printf("hb_admin_create error.\n");
        goto cleanup;
    }

    if ((retCode = hb_admin_table_exists(admin, NULL, table_name)) == 0) {
        printf("Table '%s' exists\n", table_name);
    } else if (retCode != ENOENT) {
        printf("Table '%s' not exists\n", table_name);
        goto cleanup;
    }

    retCode = hb_admin_table_enabled(admin, NULL, table_name);
    if (retCode != 0) {
        printf("hb_admin_table_enabled error.\n");
        goto cleanup;
    }

cleanup:

    if (admin) {
        hb_admin_destroy(admin, NULL, NULL);
    }

    return retCode;
}


static void hbase_put_cell (hb_client_t client)
{
    char rowkey[256];
    size_t rowkeylen;

    char family[] = "f1";
    char column[] = "author";
    char value[] = "zhangliang";

    hb_put_t put = NULL;
    hb_cell_t *cell = NULL;

    rowkeylen = snprintf(rowkey, sizeof(rowkey), "%s", "rowkey222222");

    hb_put_create(rowkey, rowkeylen, &put);

    hb_mutation_set_table(put, HTABLE_NAME, strlen(HTABLE_NAME));
    hb_mutation_set_durability(put, DURABILITY_SYNC_WAL);

    cell = (hb_cell_t *) calloc(1, sizeof(hb_cell_t));

    cell->row = rowkey;
    cell->row_len = rowkeylen;

    cell->family = family;
    cell->family_len = strlen(family);

    cell->qualifier = column;
    cell->qualifier_len = strlen(column);

    cell->value = value;
    cell->value_len = strlen(value);

    cell->ts = 1391111111112L;

    hb_put_add_cell(put, cell);

    hb_mutation_send(client, put, NULL, NULL);

    sleep(1);

    hb_client_flush(client, NULL, NULL);

    sleep(1);

    free(cell);
}


static void printRow(const hb_result_t result, void * threadid)
{
    size_t i;
    const byte_t *key = NULL;
    size_t key_len = 0;
    hb_result_get_key(result, &key, &key_len);

    size_t cell_count = 0;

    hb_result_get_cell_count(result, &cell_count);

    printf("[%p] Row='%.*s', cell count=%d\n", threadid, key_len, key, cell_count);

    const hb_cell_t **cells;
    hb_result_get_cells(result, &cells, &cell_count);

    for (i = 0; i < cell_count; ++i) {
        printf("[%p] Cell %d: family='%.*s', qualifier='%.*s', value='%.*s', timestamp=%lld.\n",
            threadid,
            i,
            cells[i]->family_len, cells[i]->family,
            cells[i]->qualifier_len, cells[i]->qualifier,
            cells[i]->value_len, cells[i]->value, cells[i]->ts);
    }
}


static void get_callback(int32_t err, hb_client_t client, hb_get_t get, hb_result_t result, void *extra)
{
    wait_done_t * done = (wait_done_t *) extra;

    if (err == 0 && result) {
        const char *table_name;
        size_t table_name_len;

        hb_result_get_table(result, &table_name, &table_name_len);

        printf("[%p] get table: {%.*s}", done->opaque, table_name_len, table_name);

        printRow(result, done->opaque);
    }
    
    if (result) {
        hb_result_destroy(result);
    }

    hb_get_destroy(get);

    wait_done_set(done, err, NULL);
}


static void hbase_get_cell (hb_client_t client, wait_done_t *getdone)
{
    char rowkey[256];
    size_t keylen;

    hb_get_t get = NULL;

    keylen = snprintf(rowkey, sizeof(rowkey), "%s", "rowkey001");

    hb_get_create(rowkey, keylen, &get);

    //hb_get_add_column(get, "f1", 2, NULL, 0);

    //hb_get_set_table(get, HTABLE_NAME, strlen(HTABLE_NAME));

    // up to ten versions of each column
    //hb_get_set_num_versions(get, 10);

    //hb_get_send(client, get, get_callback, getdone);

    printf("[%p] ...\n", (void *) pthread_self());

    hb_get_destroy(get);

    //wait_done_set(getdone, 0, NULL);
    //wait_done_until(getdone);
}


static void * get_cell_thread(void * arg)
{
    int i, retcode = -1;

    hb_client_t client = (hb_client_t) arg;
    assert(client);

    wait_done_t * getdone = wait_done_create(0, (void *) pthread_self());
    assert(getdone);

    printf("[%p] thread starting ...\n", getdone->opaque);

    for (i = 0; i < GET_COUNT_MAX; i++) {
        hbase_get_cell(client, getdone);
    }

    retcode = getdone->result;

    printf("[%p] thread exit(%d).\n", getdone->opaque, retcode);

    wait_done_discard(getdone);

    return (void *) (0);
}


int main (int argc, char *argv[])
{
    int i, rc = 0;

    hb_connection_t connection = NULL;
    hb_client_t client = NULL;
    hb_put_t put = NULL;

    pthread_t get_threads[GET_THREADS];

    printf("hello hbase start (threads=%d)...\n", GET_THREADS);

    if (argc > 1) {
        printf("ZK_QUORUM='%s'\n", argv[1]);

        rc = hb_connection_create(argv[1], ZK_ZNODE_ROOT, &connection);
    } else {
        printf("ZK_QUORUM='%s'\n", ZK_QUORUM);

        rc = hb_connection_create(ZK_QUORUM, ZK_ZNODE_ROOT, &connection);
    }

    if (rc == 0) {
        printf("hb_connection_create success.\n");
    } else {
        printf("hb_connection_create error.\n");
        goto cleanup;
    }

    /* not suitable for hbase-2
    ensure_hbase_table(connection, HTABLE_NAME);
    */

    hb_client_create(connection, &client);
    if (rc == 0) {
        printf("hb_client_create success.\n");
    } else {
        printf("hb_client_create error.\n");
        goto cleanup;
    }

    time_t t0 = time(0);
    printf("start: %ju\n", t0);

    for (i = 0; i < GET_THREADS; i++) {
        if (pthread_create(&get_threads[i], NULL, (void *) get_cell_thread, (void*) client) != 0) {
            perror("pthread_create");
            exit(-1);
        }

    }

    for (i = 0; i < GET_THREADS; i++) {
        void *res;

        if (pthread_join(get_threads[i], (void **) &res) != 0) {
            perror("pthread_join");
        }
    }

    hbase_put_cell(client);

cleanup:
    if (client) {
        hb_client_destroy(client, NULL, NULL);
    }

    if (connection) {
        hb_connection_destroy(connection);
    }

    time_t t1 = time(0);
    printf("cleanup: %ju. elapsed time=%ju. total get calls=%ju. avg %.0lf/second.\n",
        t1, t1 - t0, GET_THREADS * GET_COUNT_MAX, GET_THREADS * GET_COUNT_MAX / (t1 - t0 + 0.1));

    return rc;
}

#ifdef __cplusplus
}
#endif
