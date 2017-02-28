#
# Autogenerated by Thrift
#
# DO NOT EDIT UNLESS YOU ARE SURE THAT YOU KNOW WHAT YOU ARE DOING
#  @generated
#
from libcpp.memory cimport shared_ptr, make_shared, unique_ptr, make_unique
from libcpp.string cimport string
from libcpp cimport bool as cbool
from cpython cimport bool as pbool
from libc.stdint cimport int8_t, int16_t, int32_t, int64_t
from libcpp.vector cimport vector as vector
from libcpp.set cimport set as cset
from libcpp.map cimport map as cmap
from cython.operator cimport dereference as deref
from cpython.ref cimport PyObject
from thrift.py3.client cimport EventBase, make_py3_client, py3_get_exception
from thrift.py3.client import get_event_base
from thrift.py3.folly cimport cFollyEventBase, cFollyTry, cFollyUnit, c_unit

import asyncio
import sys
import traceback

cimport hsmodule.types
import hsmodule.types

from hsmodule.clients_wrapper cimport move

from hsmodule.clients_wrapper cimport cHsTestServiceAsyncClient, cHsTestServiceClientWrapper


cdef void HsTestService_init_callback(
        PyObject* future,
        cFollyTry[int64_t] result) with gil:
    cdef object pyfuture = <object> future
    cdef int64_t citem
    if result.hasException():
        try:
            result.exception().throwException()
        except:
            pyfuture.loop.call_soon_threadsafe(pyfuture.set_exception, sys.exc_info()[1])
    else:
        citem = result.value();
        pyfuture.loop.call_soon_threadsafe(pyfuture.set_result, citem)


cdef class HsTestService:

    def __init__(self, *args, **kwds):
        raise TypeError('Use HsTestService.connect() instead.')

    def __cinit__(self, loop):
        self.loop = loop

    @staticmethod
    cdef _hsmodule_HsTestService_set_client(HsTestService inst, shared_ptr[cHsTestServiceClientWrapper] c_obj):
        """So the class hierarchy talks to the correct pointer type"""
        inst._hsmodule_HsTestService_client = c_obj

    @staticmethod
    async def connect(str host, int port, loop=None):
        loop = loop or asyncio.get_event_loop()
        future = loop.create_future()
        future.loop = loop
        eb = await get_event_base(loop)
        cdef string _host = host.encode('UTF-8')
        make_py3_client[cHsTestServiceAsyncClient, cHsTestServiceClientWrapper](
            (<EventBase> eb)._folly_event_base,
            _host,
            port,
            0,
            made_HsTestService_py3_client_callback,
            future)
        return await future

    def init(
            self,
            arg_int1):
        future = self.loop.create_future()
        future.loop = self.loop

        deref(self._hsmodule_HsTestService_client).init(
            arg_int1,
            HsTestService_init_callback,
            future)
        return future


cdef void made_HsTestService_py3_client_callback(
        PyObject* future,
        cFollyTry[shared_ptr[cHsTestServiceClientWrapper]] result) with gil:
    cdef object pyfuture = <object> future
    if result.hasException():
        try:
            result.exception().throwException()
        except:
            pyfuture.loop.call_soon_threadsafe(pyfuture.set_exception, sys.exc_info()[1])
    else:
        pyclient = <HsTestService> HsTestService.__new__(HsTestService, pyfuture.loop)
        HsTestService._hsmodule_HsTestService_set_client(pyclient, result.value())
        pyfuture.loop.call_soon_threadsafe(pyfuture.set_result, pyclient)

