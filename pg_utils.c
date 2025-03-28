#include "pg_utils.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define LOG_FILE "pg_utils.log"

void pg_log(LogLevel level, const char *format, ...) {
    FILE *log = fopen(LOG_FILE, "a");
    if (!log) return;

    time_t now = time(NULL);
    char *level_str = (level == LOG_INFO)    ? "INFO" :
                      (level == LOG_WARNING) ? "WARN" :
                                               "ERROR";

    va_list args;
    va_start(args, format);

    fprintf(log, "[%s] [%s] ", ctime(&now), level_str);
    vfprintf(log, format, args);
    fputc('\n', log);

    va_end(args);
    fclose(log);
}

PGconn* pg_connect(PGConnParams *params) {
    const char *keywords[] = {
        "host", "port", "dbname", "user", "password", NULL
    };
    const char *values[] = {
        params->host, params->port, params->dbname,
        params->user, params->password, NULL
    };

    pg_log(LOG_INFO, "Попытка подключения к %s@%s:%s/%s",
           params->user, params->host, params->port, params->dbname);

    PGconn *conn = PQconnectdbParams(keywords, values, 0);
    if (PQstatus(conn) != CONNECTION_OK) {
        pg_log(LOG_ERROR, "Ошибка подключения: %s", PQerrorMessage(conn));
        PQfinish(conn);
        return NULL;
    }

    pg_log(LOG_INFO, "успешное подключение к базе данных");
    return conn;
}

void pg_disconnect(PGconn *conn) {
    pg_log(LOG_INFO, "ззакрытие соединения");
    PQfinish(conn);
}

int pg_test_connection(PGconn *conn) {
    pg_log(LOG_INFO, "Тестирование подключения...");
    PGresult *res = PQexec(conn, "SELECT 1");

    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        pg_log(LOG_ERROR, "Тест подключения провален: %s", PQerrorMessage(conn));
        PQclear(res);
        return 0;
    }

    PQclear(res);
    pg_log(LOG_INFO, "Тест подключения пройден");
    return 1;
}

int pg_create_backup(PGconn *conn, BackupFormat format, const char *backup_path) {
    char command[1024];
    const char *format_str = (format == FORMAT_SQL)  ? "plain" :
                             (format == FORMAT_TAR)  ? "tar" :
                                                       "custom";

    snprintf(command, sizeof(command),
        "pg_dump -h %s -p %s -U %s -F %s -f %s %s 2>&1",
        PQhost(conn), PQport(conn), PQuser(conn),
        format_str, backup_path, PQdb(conn));

    pg_log(LOG_INFO, "Создание backup: %s", command);

    FILE *pipe = popen(command, "r");
    if (!pipe) {
        pg_log(LOG_ERROR, "Не удалось запустить pg_dump");
        return 0;
    }

    char buffer[128];
    while (fgets(buffer, sizeof(buffer), pipe)) {
        pg_log(LOG_INFO, "pg_dump: %s", buffer);
    }

    int result = pclose(pipe);
    if (result != 0) {
        pg_log(LOG_ERROR, "Ошибка создания бэкапа. Код: %d", result);
        return 0;
    }

    pg_log(LOG_INFO, "BackUp успешно создан: %s", backup_path);
    return 1;
}

int pg_restore_backup(PGConnParams *params, BackupFormat format, const char *backup_path) {
    char command[1024];

    if (format == FORMAT_SQL) {
        snprintf(command, sizeof(command),
            "psql -h %s -p %s -U %s -d %s -f %s 2>&1",
            params->host, params->port, params->user,
            params->dbname, backup_path);
    } else {
        const char *format_str = (format == FORMAT_TAR) ? "--format=t" : "--format=c";
        snprintf(command, sizeof(command),
            "pg_restore %s -h %s -p %s -U %s -d %s -C %s 2>&1",
            format_str, params->host, params->port, params->user,
            params->dbname, backup_path);
    }

    pg_log(LOG_INFO, "Восстановление из бэкапа: %s", command);

    FILE *pipe = popen(command, "r");
    if (!pipe) {
        pg_log(LOG_ERROR, "Восстановление не удалось");
        return 0;
    }

    char buffer[128];
    while (fgets(buffer, sizeof(buffer), pipe)) {
        pg_log(LOG_INFO, "pg_restore: %s", buffer);
    }

    int result = pclose(pipe);
    if (result != 0) {
        pg_log(LOG_ERROR, "Ошибка восстановления. Код: %d", result);
        return 0;
    }

    pg_log(LOG_INFO, "Восстановление завершено успешно");
    return 1;
}