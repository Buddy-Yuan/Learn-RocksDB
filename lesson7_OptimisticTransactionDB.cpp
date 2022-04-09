#include <cassert>
#include <rocksdb/db.h>
#include <iostream>
#include <rocksdb/utilities/transaction.h>
#include "rocksdb/utilities/optimistic_transaction_db.h"

using namespace std;
using ROCKSDB_NAMESPACE::DB;
using ROCKSDB_NAMESPACE::OptimisticTransactionDB;
using ROCKSDB_NAMESPACE::OptimisticTransactionOptions;
using ROCKSDB_NAMESPACE::Transaction;
using ROCKSDB_NAMESPACE::WriteOptions;
using ROCKSDB_NAMESPACE::ReadOptions;
using ROCKSDB_NAMESPACE::Status;

//定义一个函数，输出值
void check_status(rocksdb::Status s,std::string value);

int main(int argc, char** argv) {

  rocksdb::Options options;
  options.create_if_missing = true;
  DB* db;
  OptimisticTransactionDB* txn_db;

  //打开数据库，之前打开数据库的方式和现在不同，现在使用的是 OptimisticTransactionDB
  Status s = OptimisticTransactionDB::Open(options, "/tmp/testdb", &txn_db);
  assert(s.ok());
  db = txn_db->GetBaseDB();

  WriteOptions write_options;
  ReadOptions read_options;
  OptimisticTransactionOptions txn_options;
  std::string value;

  //开启一个事务
  Transaction* txn = txn_db->BeginTransaction(write_options);
  assert(txn);

  //写一个key进去
  s = txn->Put("abc", "xyz");
  s = txn->Commit();

  s = txn_db->Get(read_options, "abc", &value);
  check_status(s,value);
  
  //关闭数据库
  s= txn_db->Close();
  delete txn_db;
  ROCKSDB_NAMESPACE::DestroyDB("/tmp/testdb", options);
}

void check_status(rocksdb::Status s,std::string value)
{
  if(s.ok())
  {
    cout<<"the value of "<<value<<endl; 

  }else
  {
    cout<<"the value is not exists"<<endl; 
  }

}
