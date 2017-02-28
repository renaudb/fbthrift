/**
 * Autogenerated by Thrift
 *
 * DO NOT EDIT UNLESS YOU ARE SURE THAT YOU KNOW WHAT YOU ARE DOING
 *  @generated
 */

#include <src/gen-py3/module/clients_wrapper.h>

namespace cpp2 {
RaiserClientWrapper::RaiserClientWrapper(
    std::shared_ptr<cpp2::RaiserAsyncClient> async_client,
    std::shared_ptr<folly::EventBase> event_base) : 
    async_client(async_client),
    event_base(event_base) {}

RaiserClientWrapper::~RaiserClientWrapper() {}

void RaiserClientWrapper::doBland(
    std::function<void(PyObject*, folly::Try<folly::Unit>)> callback,
    PyObject* py_future) {
  async_client->future_doBland(
  ).via(event_base.get()).then(
    [=] (folly::Try<folly::Unit>&& result) {
      callback(py_future, result);
    }
  );
}

void RaiserClientWrapper::doRaise(
    std::function<void(PyObject*, folly::Try<folly::Unit>)> callback,
    PyObject* py_future) {
  async_client->future_doRaise(
  ).via(event_base.get()).then(
    [=] (folly::Try<folly::Unit>&& result) {
      callback(py_future, result);
    }
  );
}

void RaiserClientWrapper::get200(
    std::function<void(PyObject*, folly::Try<std::string>)> callback,
    PyObject* py_future) {
  async_client->future_get200(
  ).via(event_base.get()).then(
    [=] (folly::Try<std::string>&& result) {
      callback(py_future, result);
    }
  );
}

void RaiserClientWrapper::get500(
    std::function<void(PyObject*, folly::Try<std::string>)> callback,
    PyObject* py_future) {
  async_client->future_get500(
  ).via(event_base.get()).then(
    [=] (folly::Try<std::string>&& result) {
      callback(py_future, result);
    }
  );
}


} // namespace cpp2
