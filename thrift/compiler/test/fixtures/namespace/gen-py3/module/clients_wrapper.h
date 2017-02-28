/**
 * Autogenerated by Thrift
 *
 * DO NOT EDIT UNLESS YOU ARE SURE THAT YOU KNOW WHAT YOU ARE DOING
 *  @generated
 */

#pragma once
#include <src/gen-cpp2/TestService.h>

#include <folly/Try.h>
#include <folly/Unit.h>
#include <folly/io/async/EventBase.h>

#include <Python.h>

#include <cstdint>
#include <functional>
#include <map>
#include <memory>
#include <set>
#include <vector>

namespace cpp2 {

class TestServiceClientWrapper {
  protected:
    std::shared_ptr<cpp2::TestServiceAsyncClient> async_client;
    std::shared_ptr<folly::EventBase> event_base;
  public:
    explicit TestServiceClientWrapper(
      std::shared_ptr<cpp2::TestServiceAsyncClient> async_client,
      std::shared_ptr<folly::EventBase> event_base);
    virtual ~TestServiceClientWrapper();
    void init(
      int64_t arg_int1,
      std::function<void(PyObject*, folly::Try<int64_t>)> callback,
      PyObject* py_future);
};


} // namespace cpp2
