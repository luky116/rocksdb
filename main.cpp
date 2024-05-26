//
// Created by  on 2024/1/11.
//
#include <iostream>
#include <string>

#include "rocksdb/db.h"
#include "rocksdb/options.h"
#include "rocksdb/slice.h"

using ROCKSDB_NAMESPACE::DB;
using ROCKSDB_NAMESPACE::Options;
using ROCKSDB_NAMESPACE::PinnableSlice;
using ROCKSDB_NAMESPACE::ReadOptions;
using ROCKSDB_NAMESPACE::Status;
using ROCKSDB_NAMESPACE::WriteBatch;
using ROCKSDB_NAMESPACE::WriteOptions;

// 参考资料：https://blog.csdn.net/dlf123321/article/details/135534752

// rocksdb存储路径
std::string kDBPath="c://myrocksdb";

int main()
{
  DB* db;
  Options options;
  options.IncreaseParallelism();
  //文件夹没有数据就创建
  options.create_if_missing=true;
  // 打开数据库，加载数据到内存
  Status s=DB::Open(options,kDBPath,&db);
  assert(s.ok());
  // 写key-value
  s=db->Put(WriteOptions(),"key01","value");
  assert(s.ok());

  std::string value;
  s=db->Get(ReadOptions(),"key01",&value);
  assert(s.ok());
  std::cout<<"get value  "<< value<<std::endl;
  assert(value=="value");
  // 管道，原子方式更新
  {
    WriteBatch batch;
    batch.Delete("key01");
    batch.Put("key02",value);
    s=db->Write(WriteOptions(),&batch);
  }
  s=db->Get(ReadOptions(),"key01",&value);
  assert(s.IsNotFound());
  s=db->Get(ReadOptions(),"key02",&value);
  assert(value=="value");

  {
    PinnableSlice pinnable_val;
    // 列族方式读取
    db->Get(ReadOptions(),db->DefaultColumnFamily(),"key02",&pinnable_val);
    assert(pinnable_val=="value");
  }

  {
    std::string string_val;
    PinnableSlice pinnable_val(&string_val);
    // 列族方式读取
    db->Get(ReadOptions(),db->DefaultColumnFamily(),"key02",&pinnable_val);
    assert(pinnable_val=="value");
    assert(pinnable_val.IsPinned() || string_val == "value");
  }

  PinnableSlice pinnable_val;
  s=db->Get(ReadOptions(),db->DefaultColumnFamily(),"key01",&pinnable_val);
  assert(s.IsNotFound());

  pinnable_val.Reset();
  db->Get(ReadOptions(),db->DefaultColumnFamily(),"key02",&pinnable_val);
  assert(pinnable_val=="value");
  pinnable_val.Reset();

  delete db;

  return 0;
}
