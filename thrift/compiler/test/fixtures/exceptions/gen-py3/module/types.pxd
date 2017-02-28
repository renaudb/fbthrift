#
# Autogenerated by Thrift
#
# DO NOT EDIT UNLESS YOU ARE SURE THAT YOU KNOW WHAT YOU ARE DOING
#  @generated
#

from libcpp.string cimport string
from libcpp cimport bool as cbool
from cpython cimport bool as pbool
from libc.stdint cimport int8_t, int16_t, int32_t, int64_t
from libcpp.memory cimport shared_ptr, unique_ptr
from libcpp.vector cimport vector
from libcpp.set cimport set as cset
from libcpp.map cimport map as cmap, pair as cpair
from thrift.py3.exceptions cimport cTException
cimport thrift.py3.types




cdef extern from "src/gen-cpp2/module_types.h" namespace "cpp2":
    cdef cppclass cBanal__isset "cpp2::Banal::__isset":

    # Forward Declaration
    cdef cppclass cBanal "cpp2::Banal"(cTException)

    cdef cppclass cBanal "cpp2::Banal"(cTException):
        cBanal() except +
        cBanal(const cBanal&) except +
        bint operator==(cBanal&)
        cBanal__isset __isset

    cdef cppclass cFiery__isset "cpp2::Fiery::__isset":
        bint message

    # Forward Declaration
    cdef cppclass cFiery "cpp2::Fiery"(cTException)

    cdef cppclass cFiery "cpp2::Fiery"(cTException):
        cFiery() except +
        cFiery(const cFiery&) except +
        bint operator==(cFiery&)
        string message
        cFiery__isset __isset


cdef extern from "<utility>" namespace "std" nogil:
    cdef shared_ptr[cBanal] move(unique_ptr[cBanal])
    cdef shared_ptr[cFiery] move(unique_ptr[cFiery])

# Forward Definition of the cython struct
cdef class Banal(thrift.py3.types.Exception)

cdef class Banal(thrift.py3.types.Exception):
    cdef object __hash
    cdef object __weakref__
    cdef shared_ptr[cBanal] c_Banal

    @staticmethod
    cdef create(shared_ptr[cBanal] c_Banal)

# Forward Definition of the cython struct
cdef class Fiery(thrift.py3.types.Exception)

cdef class Fiery(thrift.py3.types.Exception):
    cdef object __hash
    cdef object __weakref__
    cdef shared_ptr[cFiery] c_Fiery

    @staticmethod
    cdef create(shared_ptr[cFiery] c_Fiery)




