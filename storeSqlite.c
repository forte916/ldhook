#include "debug.h"
#include "storeSqlite.h"

////////////////////////////////////////////////////////////
// private
static int print_items(void* vp, int argc, char** argv, char** columns) {
	int i;
	for (i=0; i<argc; i++) {
		DEBUG_PRINT("  %s:%s ", columns[i], argv[i]);
	}
	DEBUG_PRINT("\n");
	return 0;
}


////////////////////////////////////////////////////////////
// public
sqlite3* initOpenSessionDB()
{
	int err;
	sqlite3* db = NULL;

	err = sqlite3_open(DB_PATH, &db);
	if (err != SQLITE_OK) {
		ERROR_PRINT("db open error!!:code[%d] \n", err);
		ERROR_PRINT("errmesg[%s] \n", sqlite3_errmsg(db));
		return NULL;
	}

	err = sqlite3_exec(db, STMT_CREATE_TABLE, NULL, NULL, NULL);
	if (err != SQLITE_OK) {
		ERROR_PRINT("create table error!!:code[%d] \n", err);
		ERROR_PRINT("errmesg[%s] \n", sqlite3_errmsg(db));
		return NULL;
	}

	return db;
}

int quitOpenSessionDB(sqlite3* db)
{
	int err;

	err = sqlite3_close(db);
	if (err != SQLITE_OK) {
		ERROR_PRINT("db close error!!:code[%d] \n", err);
		ERROR_PRINT("errmesg[%s] \n", sqlite3_errmsg(db));
		return -1;
	}
	return 0;
}


int putOpenSessionDB(sqlite3* db, struct store_msg* sm)
{
	DEBUG_FUNCIN("\n");
	int err;
	sqlite3_stmt *stm = NULL;

	if (sm == NULL) {
		return -1;
	}

	err = sqlite3_exec(db, "BEGIN IMMEDIATE;", NULL, NULL, NULL);
	// TODO:: error handling

	err = sqlite3_prepare(db, STMT_PUT_OPEN_SESSION, -1, &stm, NULL);
	if (err != SQLITE_OK || !stm) {
		ERROR_PRINT("prepare error!!:code[%d] \n", err);
		ERROR_PRINT("errmesg[%s] \n", sqlite3_errmsg(db));
		return -1;
	}

	storeMsgToString(sm);  // for debug
	sqlite3_bind_int(stm, 1, sm->pid);
	sqlite3_bind_int(stm, 2, sm->fd);
	sqlite3_bind_text(stm, 3, sm->path, -1, SQLITE_TRANSIENT);

	err = sqlite3_step(stm);
	if (err != SQLITE_DONE) {
		ERROR_PRINT("step error!!:code[%d] \n", err);
		ERROR_PRINT("errmesg[%s] \n", sqlite3_errmsg(db));
		return -1;
	}

	err = sqlite3_reset(stm);
	// TODO:: error handling

	err = sqlite3_clear_bindings(stm);
	// TODO:: error handling

	sqlite3_finalize(stm);

	err = sqlite3_exec(db, "COMMIT;", NULL, NULL, NULL);
	// TODO:: error handling

	DEBUG_FUNCOUT("\n");
	return 1;
}

int delOpenSessionDB(sqlite3* db, struct store_msg* sm)
{
	DEBUG_FUNCIN("\n");
	int err;
	sqlite3_stmt *stm = NULL;

	if (sm == NULL) {
		return -1;
	}

	err = sqlite3_exec(db, "BEGIN IMMEDIATE;", NULL, NULL, NULL);
	// TODO:: error handling

	err = sqlite3_prepare(db, STMT_DEL_OPEN_SESSION, -1, &stm, NULL);
	if (err != SQLITE_OK || !stm) {
		ERROR_PRINT("prepare error!!:code[%d] \n", err);
		ERROR_PRINT("errmesg[%s] \n", sqlite3_errmsg(db));
		return -1;
	}

	storeMsgToString(sm);  // for debug
	sqlite3_bind_int(stm, 1, sm->pid);
	sqlite3_bind_int(stm, 2, sm->fd);

	err = sqlite3_step(stm);
	if (err != SQLITE_DONE) {
		ERROR_PRINT("step error!!:code[%d] \n", err);
		ERROR_PRINT("errmesg[%s] \n", sqlite3_errmsg(db));
		return -1;
	}

	err = sqlite3_reset(stm);
	// TODO:: error handling

	err = sqlite3_clear_bindings(stm);
	// TODO:: error handling

	sqlite3_finalize(stm);

	err = sqlite3_exec(db, "COMMIT;", NULL, NULL, NULL);
	// TODO:: error handling

	DEBUG_FUNCOUT("\n");
	return 1;
}

int getOpenSessionDB(sqlite3* db)
{
	DEBUG_FUNCIN("\n");
	int err;
	sqlite3_stmt *stm = NULL;

	err = sqlite3_prepare(db, STMT_GET_OPEN_SESSION, -1, &stm, NULL);
	if (err != SQLITE_OK || !stm) {
		ERROR_PRINT("prepare error!!:code[%d] \n", err);
		ERROR_PRINT("errmesg[%s] \n", sqlite3_errmsg(db));
		return -1;
	}

	while (sqlite3_step(stm) == SQLITE_ROW) {
		DEBUG_PRINT(" pid:%d \n", (int) sqlite3_column_int(stm, 0));
		DEBUG_PRINT("  fd:%d \n", (int) sqlite3_column_int(stm, 1));
		DEBUG_PRINT("path:%s \n", (char *) sqlite3_column_text(stm, 2));
	}

	sqlite3_finalize(stm);

	DEBUG_FUNCOUT("\n");
	return 1;
}

