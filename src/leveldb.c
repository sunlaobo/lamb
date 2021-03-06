
/* 
 * Lamb Gateway Platform
 * Copyright (C) 2017 typefo <typefo@qq.com>
 * Update: 2017-07-20
 */

#include "db.h"

int lamb_db_init(lamb_db_t *db, const char *name) {
    db->options = leveldb_options_create();
    leveldb_options_set_create_if_missing(db->options, 1);

    if (lamb_db_open(db, name) != 0) {
        return -1;
    }

    db->roptions = leveldb_readoptions_create();
    db->woptions = leveldb_writeoptions_create();

    return 0;
}

int lamb_db_open(lamb_db_t *db, const char *name) {
    char *err = NULL;

    db->handle = leveldb_open(db->options, name, &err);

    if (err != NULL) {
        return -1;
    }

    leveldb_free(err);
    err = NULL;

    return 0;
}

int lamb_db_put(lamb_db_t *db, const char *key, size_t keylen, const char *val, size_t vallen) {
    char *err = NULL;

    leveldb_put(db->handle, db->woptions, key, keylen, val, vallen, &err);
    if (err != NULL) {
        return -1;
    }

    leveldb_free(err);
    err = NULL;

    return 0;
}

char *lamb_db_get(lamb_db_t *db, const char *key, size_t keylen, size_t *len) {
    char *val;
    char *err = NULL;

    val = leveldb_get(db->handle, db->roptions, key, keylen, len, &err);
    if (err != NULL) {
        return NULL;
    }

    leveldb_free(err);

    return val;
}

int lamb_db_delete(lamb_db_t *db, const char *key, size_t keylen) {
    char *err = NULL;

    leveldb_delete(db->handle, db->woptions, key, keylen, &err);

    if (err != NULL) {
        return -1;
    }

    leveldb_free(err);
    err = NULL;

    return 0;
}

int lamb_db_close(lamb_db_t *db) {
    char *err = NULL;

    leveldb_close(db->handle);
    leveldb_destroy_db(db->options, db->name, &err);

    if (err != NULL) {
        return -1;
    }

    leveldb_free(err);
    err = NULL;

    return 0;
}
