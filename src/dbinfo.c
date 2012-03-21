/* -*- Mode: C; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: nil -*- */
#include "config.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <libcouchstore/couch_db.h>
#include <snappy-c.h>
#include <ei.h>
#include "util.h"

static char *size_str(double size)
{
    static char rfs[256];
    int i = 0;
    const char *units[] = {"bytes", "kB", "MB", "GB", "TB", "PB", "EB", "ZB", "YB"};
    while (size > 1024) {
        size /= 1024;
        i++;
    }
    snprintf(rfs, sizeof(rfs), "%.*f %s", i, size, units[i]);
    return rfs;
}

static uint64_t id_reduce_info(node_pointer *root)
{
    int pos = 0;
    uint64_t total, deleted, size;
    if (root == NULL) {
        printf("   no documents\n");
        return 0;
    }
    ei_decode_tuple_header(root->reduce_value.buf, &pos, NULL);
    ei_decode_uint64(root->reduce_value.buf, &pos, &total);
    ei_decode_uint64(root->reduce_value.buf, &pos, &deleted);
    ei_decode_uint64(root->reduce_value.buf, &pos, &size);
    printf("   doc count: %"PRIu64"\n", total);
    printf("   deleted doc count: %"PRIu64"\n", deleted);
    printf("   data size: %s\n", size_str(size));
    return size;
}

static int process_file(const char *file)
{
    Db *db = NULL;
    couchstore_error_t errcode = couchstore_open_db(file, 0, NULL, &db);
    if (errcode != COUCHSTORE_SUCCESS) {
        fprintf(stderr, "Failed to open \"%s\": %s\n",
                file, couchstore_strerror(errcode));
        return -1;
    }

    uint64_t datasize = 0;
    uint64_t btreesize = 0;
    printf("DB Info (%s)\n", file);
    printf("   file format version: %"PRIu64"\n", db->header.disk_version);
    printf("   update_seq: %"PRIu64"\n", db->header.update_seq);
    datasize = id_reduce_info(db->header.by_id_root);
    if (db->header.by_id_root) {
        btreesize += db->header.by_id_root->subtreesize;
    }
    if (db->header.by_seq_root) {
        btreesize += db->header.by_seq_root->subtreesize;
    }
    printf("   B-tree size: %s\n", size_str(btreesize));
    printf("   total disk size: %s\n", size_str(db->file_pos));

    couchstore_close_db(db);

    return 0;
}

int main(int argc, char **argv)
{
    if (argc < 2) {
        printf("USAGE: %s <file.couch>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int error = 0;
    for (int ii = 1; ii < argc; ++ii) {
        error += process_file(argv[ii]);
    }

    if (error) {
        exit(EXIT_FAILURE);
    } else {
        exit(EXIT_SUCCESS);
    }
}