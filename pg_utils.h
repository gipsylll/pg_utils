#ifndef PG_UTILS_H
#define PG_UTILS_H

#include <libpq-fe.h>

typedef enum {
    LOG_INFO,
    LOG_WARNING,
    LOG_ERROR
} LogLevel;

typedef struct {
    char *host;
    char *port;
    char *dbname;
    char *user;
    char *password;
} PGConnParams;

typedef enum {
    FORMAT_SQL,
    FORMAT_TAR,
    FORMAT_CUSTOM
} BackupFormat;




PGconn* pg_connect(PGConnParams *params);
int pg_test_connection(PGconn *conn);
int pg_create_backup(PGconn *conn, BackupFormat format, const char *backup_path);
int pg_restore_backup(PGConnParams *params, BackupFormat format, const char *backup_path);
void pg_disconnect(PGconn *conn);
void pg_log(LogLevel level, const char *format, ...);

#endif