#include <cstring>
#include <sstream>
#include "sqlite3.h"
using namespace std;
static int callback(void *NotUsed, int argc, char **argv, char **azColName)
{
  return 0;
}

void sqlite_execute_easy(sqlite3* db, string pol, int errorcode)
{
  char *zErrMsg = 0;
  int rc;
  rc = sqlite3_exec(db, pol.c_str(), callback, 0, &zErrMsg);
  if(rc != SQLITE_OK)
  {
    //cerr << "SQL error: " << zErrMsg << endl;
    sqlite3_free(zErrMsg);
    exit(errorcode);
  }
}

const string OSM_TREE = "CREATE TABLE IF NOT EXISTS OSM_TREE("  \
	"RELATION_ID BIGINT PRIMARY KEY NOT NULL," \
	"RELATION_PARENT BIGINT," \
	"NAME CHAR(255)," \
	"TYPE CHAR(1)," \
	"ROUTE_ID CHAR(20), FOREIGN KEY(RELATION_PARENT) REFERENCES OSM_TREE(RELATION_ID));";

const string OSM_RELATIONS = "CREATE TABLE IF NOT EXISTS OSM_RELATIONS("  \
"RELATION_ID BIGINT NOT NULL," \
"REF_ID CHAR(10) NOT NULL, PRIMARY KEY(RELATION_ID, REF_ID));";

const string OSM_STOPS = "CREATE TABLE IF NOT EXISTS OSM_STOPS("  \
	"NORMAL_STOP BIGINT," \
	"STOP_POSITION BIGINT," \
	"REF_ID CHAR(10) PRIMARY KEY NOT NULL," \
	"NORMAL_STOP_NAME CHAR(255)," \
	"STOP_POSITION_NAME CHAR(255)" \
	");";

const string OPERATOR_ROUTES = "CREATE TABLE IF NOT EXISTS OPERATOR_ROUTES("  \
	"ROUTE_ID CHAR(20) NOT NULL," \
	"DIRECTION INTEGER NOT NULL," \
	"STOP_ON_DIRECTION_NUMBER INTEGER NOT NULL," \
	"STOP_ID CHAR(10), PRIMARY KEY(ROUTE_ID, DIRECTION, STOP_ON_DIRECTION_NUMBER));";


const string OPERATOR_ROUTES2 = "CREATE TABLE IF NOT EXISTS OPERATOR_ROUTES("  \
	"ROUTE_ID varchar(10) NOT NULL," \
	"DIRECTION integer NOT NULL," \
	"STOP_ON_DIRECTION_NUMBER integer NOT NULL," \
	"STOP_ID varchar(10), PRIMARY KEY(ROUTE_ID, DIRECTION, STOP_ON_DIRECTION_NUMBER));";



const string OPERATOR_STOPS = "CREATE TABLE IF NOT EXISTS OPERATOR_STOPS("  \
	"STOP_ID CHAR(10) NOT NULL PRIMARY KEY," \
	"NAME CHAR(100)," \
	"LON REAL," \
	"LAT REAL," \
	"COORDINATES_QUALITY INTEGER," \
	"MORE_INFO TEXT);";

const string OPERATOR_SCHEDULE = "CREATE TABLE IF NOT EXISTS SCHEDULE("  \
	"LINE CHAR(10)," \
	"TRIP INTEGER," \
	"NEXT_STOP_TRIP INTEGER," \
	"TIME_SECONDS INTEGER," \
	"DAY_TYPE CHAR(3), STOP_ID CHAR(6), DIRECTION CHAR(6));";

const string OPERATOR_CALENDAR = "CREATE TABLE IF NOT EXISTS OPERATOR_CALENDAR("  \
	"DATE CHAR(20)," \
	"DAY_TYPE CHAR(3));";



void create_tables_if_not_exist(sqlite3* db)
{
  sqlite_execute_easy(db, OSM_TREE, 2);
  sqlite_execute_easy(db, OSM_RELATIONS, 2);
  sqlite_execute_easy(db, OSM_STOPS, 2);
  sqlite_execute_easy(db, OPERATOR_ROUTES, 2);
  sqlite_execute_easy(db, OPERATOR_STOPS, 2);
}

void create_tables_if_not_exist_schedule(sqlite3* db)
{
  sqlite_execute_easy(db, OPERATOR_SCHEDULE, 2);
  sqlite_execute_easy(db, OPERATOR_STOPS, 2);
  sqlite_execute_easy(db, OPERATOR_CALENDAR, 2);
}

string if0(long long val)
{
  if(val == 0)
    return "NULL";
  stringstream foo;
  foo<<val;
  return foo.str();
}
