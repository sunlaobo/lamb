
/* 
 * Lamb Gateway Platform
 * Copyright (C) 2017 typefo <typefo@qq.com>
 * Update: 2017-07-14
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <pcre.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/prctl.h>
#include <time.h>
#include <sys/time.h>
#include <pthread.h>
#include <iconv.h>
#include <cmpp.h>
#include "utils.h"

int lamb_signal(int sig, void (*handler)(int)) {
    struct sigaction sa;

    sa.sa_handler = handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    if (sigaction(sig, &sa, NULL) == -1) {
        return -1;
    }

    return 0;
}

void lamb_signal_processing(void) {
    int err;
    
    err = lamb_signal(SIGCHLD, SIG_IGN);
    if (err) {
        fprintf(stderr, "setting SIGCHLD signal processor failed\n");
    }
    
    err = lamb_signal(SIGPIPE, SIG_IGN);
    if (err) {
        fprintf(stderr, "setting SIGPIPE signal processor failed\n");
    }
    
    return;
}

void lamb_daemon(void) {
    int fd, maxfd;
    switch (fork()) {
    case -1:
        return;
    case 0:
        break;
    default:
        _exit(EXIT_SUCCESS);
    }

    if (setsid() == -1) {
        return;
    }

    switch (fork()) {
    case -1:
        return;
    case 0:
        break;
    default:
        _exit(EXIT_SUCCESS);
    }

    umask(0);
    chdir("/");

    maxfd = sysconf(_SC_OPEN_MAX);
    if (maxfd == -1) {
        maxfd = 8192;
        for (fd = 0; fd < maxfd; fd++) {
            close(fd);
        }
    }

    close(STDIN_FILENO);
    fd = open("/dev/null", O_RDWR);
    if (fd != STDIN_FILENO) {
        return;
    }

    if (dup2(STDIN_FILENO, STDOUT_FILENO) != STDOUT_FILENO) {
        return;
    }

    if (dup2(STDIN_FILENO, STDERR_FILENO) != STDERR_FILENO) {
        return;
    }

    return;
}

void lamb_sleep(unsigned long long milliseconds) {
    struct timeval timeout;

    timeout.tv_sec = milliseconds / 1000;
    timeout.tv_usec = (milliseconds * 1000) % 1000000;
    select(0, NULL, NULL, NULL, &timeout);

    return;
}

void lamb_msleep(unsigned long long microsecond) {
    struct timeval timeout;

    timeout.tv_sec = microsecond / 1000000;
    timeout.tv_usec = microsecond  % 1000000;
    select(0, NULL, NULL, NULL, &timeout);

    return;
}

unsigned long long lamb_now_microsecond(void) {
    unsigned long long now;
    struct timeval tv;

    gettimeofday(&tv, NULL);
    now = (unsigned long long)tv.tv_sec * 1000000 + (unsigned long long)tv.tv_usec;
    return now;
}

void lamb_flow_limit(unsigned long long *start, unsigned long long *now, unsigned long long *next, int *count, int limit) {
    if (limit < 1) {
        return;
    }
    
    if (*now > *next) {
        *next += 1000000;
    }

    while (((*count * 1000000.0) / (*now - *start)) > limit) {
        lamb_msleep(1000);
        *now = lamb_now_microsecond();
    }

    if ((*count % 8) == 0) {
        *count = 0;
        *start = lamb_now_microsecond();
        *next = *start + 1000000;
    }

    return;
}

void lamb_log_error(const char *logfile, char *file, int line, const char *fmt, ...) {
    char buff[512];
    time_t rawtime;
    struct tm *t;

    time(&rawtime);
    t = localtime(&rawtime);
    sprintf(buff, "[%4d-%02d-%02d %02d:%02d:%02d] %s:%d %s\n", t->tm_year + 1900,
            t->tm_mon + 1, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec, file, line, fmt);

    FILE *fp = NULL;
    fp = fopen(logfile, "a");
    if (fp != NULL) {
        va_list ap;
        va_start(ap, fmt);
        vfprintf(fp, buff, ap);
        va_end(ap);
        fclose(fp);
    }

    return;
}

unsigned short lamb_sequence(void) {
    static unsigned short seq = 1;
    return (seq < 0xffff) ? (seq++) : (seq = 1);
}

char *lamb_strdup(const char *str) {
    size_t len = strlen (str) + 1;
    void *new = malloc (len);

    if (new == NULL) {
        return NULL;
    }

    return (char *) memcpy (new, str, len);
}

void lamb_start_thread(void *(*func)(void *), void *arg, int count) {
    int err;
    pthread_t tid;
    pthread_attr_t attr;

    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    for (int i = 0; i < count; i++) {
        err = pthread_create(&tid, &attr, func, arg);
        if (err) {
            continue;
        }
    }

    pthread_attr_destroy(&attr);
    return;
}

unsigned long long lamb_gen_msgid(int gid, unsigned short sequenceId) {
    time_t rawtime;
    struct tm *t;
    unsigned long long msgId;

    time(&rawtime);
    t = localtime(&rawtime);

    /* Generate Message ID */
    msgId = cmpp_gen_msgid(t->tm_mon + 1, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec, gid, sequenceId);

    return msgId;
}

void lamb_set_process(char *name) {
    prctl(PR_SET_NAME, name, 0, 0, 0);
    return;
}

bool lamb_pcre_regular(char *pattern, char *content, int len) {
    int rc;
    pcre *re;
    const char *error;
    int erroffset;
    int ovector[510];

    re = pcre_compile(pattern, 0, &error, &erroffset, NULL);
    if (re == NULL) {
        return false;
    }

    rc = pcre_exec(re, NULL, content, len, 0, 0, ovector, 510);
    if (rc < 0) {
        pcre_free(re);
        return false;
    }

    pcre_free(re);
    return true;
}

int lamb_mqd_writable(int fd, long long millisecond) {
    int ret;
    fd_set wset;
    struct timeval timeout;

    timeout.tv_sec = millisecond / 1000;
    timeout.tv_usec = (millisecond % 1000) * 1000;

    FD_ZERO(&wset);
    FD_SET(fd, &wset);
    ret = select(fd + 1, NULL, &wset, NULL, (millisecond != 0) ? &timeout : 0);

    return ret;
}

int lamb_encoded_convert(const char *src, size_t slen, char *dst, size_t dlen, const char* fromcode, const char* tocode, int *length) {
    iconv_t cd;
    size_t len = dlen;
    char *inbuf = (char *)src;
    size_t *inbytesleft = &slen;
    char *outbuf = dst;
    size_t *outbytesleft = &dlen;


    cd = iconv_open(tocode, fromcode);
    if (cd != (iconv_t)-1) {
        iconv(cd, &inbuf, inbytesleft, &outbuf, outbytesleft);
        *length = len - *outbytesleft;
        iconv_close(cd);
        return 0;
    }

    return -1;
}

size_t lamb_ucs2_strlen(const char *str) {
    int i = 0;

    while (i < 140) {
        if ((str[i] + str[i + 1]) != 0) {
            i += 2;
        } else {
            break;
        }
    }

    return i;
}

size_t lamb_gbk_strlen(const char *str) {  
    const char *ptr = str;
    while (*ptr) {
        if ((*ptr < 0) && (*(ptr + 1) < 0 || *(ptr + 1) > 63)) {
            str++;
            ptr += 2;
        } else {
            ptr++;  
        }  
    }

    return (size_t)(ptr - str);
}

bool lamb_check_msgfmt(int coded, int list[], size_t len) {
    for (int i = 0; i < len; i++) {
        if (list[i] == coded) {
            return true;
        }
    }

    return false;
}
