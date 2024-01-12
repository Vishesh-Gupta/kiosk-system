#include "db.h"

#include <iostream>

// TODO: Parse the database configuration
DB::DB(){}


void DB::query(std::string sql)
{
  pqxx::connection c{"postgresql://postgres:password@localhost:5432/postgres"};
  pqxx::work txn{c};

  txn.query(sql);

  txn.commit();
}

pqxx::result DB::exec(std::string sql) {
  pqxx::connection c{"postgresql://postgres:password@localhost:5432/postgres"};
  pqxx::work txn{c};

  pqxx::result r{txn.exec(sql)};

  txn.commit();
  return r;
}
