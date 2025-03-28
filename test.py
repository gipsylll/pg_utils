import os
import ctypes
from ctypes import c_char_p, Structure


class PGConnParams(Structure):
    _fields_ = [
        ("host", c_char_p),
        ("port", c_char_p),
        ("dbname", c_char_p),
        ("user", c_char_p),
        ("password", c_char_p),
    ]


try:
    lib = ctypes.CDLL("./build/libpgutils.so")
except OSError as e:
    print(f"Ошибка загрузки библиотеки: {e}")
    exit(1)


LOG_FILE = "pg_utils.log"
BACKUP_PATH = b"./backups/local_backup.sql"
RESTORE_PATH = b"./restore/test.dump"
PARAMS = PGConnParams(
    host=b"localhost",
    port=b"5432",
    dbname=b"testdb",
    user=b"user",
    password=b"password",
)


def main():
    try:
        print("=> Подключение к БД...")
        conn = lib.pg_connect(ctypes.byref(PARAMS))
        if not conn:
            raise Exception("Ошибка подключения")

        print("=> Тестирование подключения...")
        if not lib.pg_test_connection(conn):
            raise Exception("Тест подключения провален")

        print("=> Создание бэкапа...")
        if not lib.pg_create_backup(conn, 0, BACKUP_PATH):
            raise Exception("Ошибка создания бэкапа")

        print("=> Восстановление из бэкапа...")
        if not lib.pg_restore_backup(ctypes.byref(PARAMS), 0, RESTORE_PATH):
            raise Exception("Ошибка восстановления")

        print("Все операции выполнены успешно")

    except Exception as e:
        print(f"Ошибка: {e}")
    finally:
        print(f"Логи доступны в: {os.path.abspath(LOG_FILE)}")
        if "conn" in locals():
            lib.pg_disconnect(conn)


if __name__ == "__main__":
    main()
