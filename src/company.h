
/* 
 * Lamb Gateway Platform
 * Copyright (C) 2017 typefo <typefo@qq.com>
 * Update: 2017-07-14
 */

#ifndef _LAMB_COMPANY_H
#define _LAMB_COMPANY_H

typedef struct {
    int id;
    int paytype;
} lamb_company_t;

int lamb_company_get_all(lamb_db_t *db, lamb_company_t *companys[], size_t size);

#endif