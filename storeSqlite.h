#ifndef STORE_SQLITE_H
#define STORE_SQLITE_H

#include <sqlite3.h>
#include "storeMessage.h"

////////////////////////////////////////////////////////////
// define
#define DB_PATH ":memory:"
//#define DB_PATH "opensession.db"  // for debug

#define STMT_CREATE_TABLE "CREATE TABLE opensession (" \
							"  pid INTEGER NOT NULL," \
							"  fd INTEGER NOT NULL," \
							"  path TEXT," \
							"  PRIMARY KEY (pid, fd)" \
							");"


#define STMT_PUT_OPEN_SESSION "INSERT INTO opensession VALUES(?,?,?)"
#define STMT_DEL_OPEN_SESSION "DELETE FROM opensession WHERE pid = ? and fd = ?"
#define STMT_GET_OPEN_SESSION "SELECT * FROM opensession"


sqlite3* initOpenSessionDB();
int quitOpenSessionDB(sqlite3* db);
int putOpenSessionDB(sqlite3* db, struct store_msg* sm);
int delOpenSessionDB(sqlite3* db, struct store_msg* sm);
int getOpenSessionDB(sqlite3* db);

#endif //#ifndef STORE_SQLITE_H
