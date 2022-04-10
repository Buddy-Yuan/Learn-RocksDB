#include <cassert>
#include <rocksdb/db.h>
#include <iostream>
#include <rocksdb/utilities/transaction.h>
#include <rocksdb/utilities/transaction_db.h>

using namespace std;
using ROCKSDB_NAMESPACE::TransactionDB;
using ROCKSDB_NAMESPACE::TransactionDBOptions;
using ROCKSDB_NAMESPACE::TransactionOptions;
using ROCKSDB_NAMESPACE::Transaction;
using ROCKSDB_NAMESPACE::WriteOptions;
using ROCKSDB_NAMESPACE::ReadOptions;
using ROCKSDB_NAMESPACE::Status;


int main(int argc, char** argv) {

  rocksdb::Options options;
  options.create_if_missing = true;
  TransactionDB* txn_db;
  TransactionDBOptions txn_db_options;
  WriteOptions write_options; 
  ReadOptions read_options;
  std::string value;
  vector<string> values;

  //打开数据库，之前打开数据库的方式和现在不同，现在使用的是TransactionDB 和 TransactionDBOptions
  rocksdb::Status s = TransactionDB::Open(options, txn_db_options, "/tmp/testdb", &txn_db);
  assert(s.ok());

  //开启一个事务
  Transaction* txn = txn_db->BeginTransaction(write_options);
  assert(txn);
  
  //事务外写入
  txn_db->Put(write_options, "a", "old");
  txn_db->Put(write_options, "b", "old");
  
  //事务写入,此时修改了key:"a"的值
  txn->Put("a", "new");
  
  //使用迭代器获取所有key和value
  rocksdb::Iterator* it = txn_db->NewIterator(rocksdb::ReadOptions());
  for (it->SeekToFirst(); it->Valid(); it->Next()) {
    cout << it->key().ToString() << ": " << it->value().ToString() << endl;
  }

  //使用事务迭代器
  auto it1 = txn->GetIterator(rocksdb::ReadOptions());
  for (it1->SeekToFirst(); it1->Valid(); it1->Next()) {
    cout << it1->key().ToString() << ": " << it1->value().ToString() << endl;
  }

  assert(it->status().ok()); 
  delete it;
  assert(it1->status().ok()); 
  delete it1;

  //提交事务
  s = txn->Commit();
  delete txn;

  //关闭数据库
  s= txn_db->Close();
  delete txn_db;
  ROCKSDB_NAMESPACE::DestroyDB("/tmp/testdb", options);
}
