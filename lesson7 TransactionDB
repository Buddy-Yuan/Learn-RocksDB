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

//定义一个函数，输出值
void check_status(rocksdb::Status s,std::string value);

int main(int argc, char** argv) {

  rocksdb::Options options;
  options.create_if_missing = true;
  TransactionDB* txn_db;
  TransactionDBOptions txn_db_options;
  WriteOptions write_options; 
  ReadOptions read_options;
  std::string value;

  //打开数据库，之前打开数据库的方式和现在不同，现在使用的是TransactionDB 和 TransactionDBOptions
  rocksdb::Status s = TransactionDB::Open(options, txn_db_options, "/tmp/testdb", &txn_db);
  assert(s.ok());

  //开启一个事务
  Transaction* txn = txn_db->BeginTransaction(write_options);
  assert(txn);

  //在这个事务中读取abc这个key，此时abc不存在，输出不存在
  s = txn->Get(read_options, "abc", &value);
  check_status(s,value);
  
  //事务txn写一个key
  s = txn->Put("abc", "def");
  assert(s.ok());

  // 通过TransactionDB::Get在事务外读一个key，不影响txn，因为tnx没提交，所以txn_db这里读不到
  s = txn_db->Get(read_options, "abc", &value);
  check_status(s,value);

  //在事务外写一个key。不影响 txn，因为这是一个不相同的key
  s = txn_db->Put(write_options, "xyz", "zzz");
  assert(s.ok());

  //在事务外写一个key。这个地方会冲突，因为写入了和txn相同的key
  s = txn_db->Put(write_options, "abc", "def");
  check_status(s,value);
  assert(s.subcode() == Status::kLockTimeout);

  //值xyz已经提交，在txn这个事务中可以读到。
  s = txn->Get(read_options, "xyz", &value);
  check_status(s,value);

  //提交事务
  s = txn->Commit();
  delete txn;

  //事务已经提交了，txn_db这里可以读取到。
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
