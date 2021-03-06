
/* 
 * Lamb Gateway Platform
 * Copyright (C) 2017 typefo <typefo@qq.com>
 * Update: 2017-08-24
 */

#include <stdlib.h>
#include <string.h>
#include <pcre.h>
#include "utils.h"
#include "template.h"

int lamb_template_get(lamb_db_t *db, int id, lamb_template_t *template) {
    char *column;
    char sql[256];
    PGresult *res = NULL;

    column = "id, name, contents, account";
    sprintf(sql, "SELECT %s FROM template WHERE id = %d", column, id);
    res = PQexec(db->conn, sql);
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        PQclear(res);
        return 1;
    }

    if (PQntuples(res) < 1) {
        return 2;
    }

    template->id = atoi(PQgetvalue(res, 0, 0));
    strncpy(template->name, PQgetvalue(res, 0, 1), 63);
    strncpy(template->contents, PQgetvalue(res, 0, 2), 511);
    template->account = atoi(PQgetvalue(res, 0, 3));
    
    PQclear(res);

    return 0;
}

int lamb_template_get_all(lamb_db_t *db, int id, lamb_templates_t *templates, int size) {
    int rows = 0;
    char *column;
    char sql[256];
    PGresult *res = NULL;
    
    templates->len = 0;
    column = "id, name, contents, account";
    sprintf(sql, "SELECT %s FROM template WHERE account = %d ORDER BY id", column, id);
    res = PQexec(db->conn, sql);
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        PQclear(res);
        return -1;
    }

    rows = PQntuples(res);

    for (int i = 0, j = 0; (i < rows) && (i < size); i++, j++) {
        lamb_template_t *t = NULL;
        t = (lamb_template_t *)calloc(1, sizeof(lamb_template_t));
        if (t != NULL) {
            t->id = atoi(PQgetvalue(res, i, 0));
            strncpy(t->name, PQgetvalue(res, i, 1), 63);
            strncpy(t->contents, PQgetvalue(res, i, 2), 511);
            t->account = atoi(PQgetvalue(res, i, 3));
            templates->list[i] = t;
            templates->len++;
        } else {
            if (j > 0) {
                j--;
            }
        }
    }

    PQclear(res);
    return 0;
}

bool lamb_template_check(lamb_templates_t *templates, char *content, int len) {
    char pattern[512];

    for (int i = 0; (i < templates->len) && (templates->list[i] != NULL); i++) {
        memset(pattern, 0, sizeof(pattern));
        sprintf(pattern, "【%s】%s", templates->list[i]->name, templates->list[i]->contents);
        if (lamb_pcre_regular(pattern, content, len)) {
            return true;
        }
    }

    return false;
}

