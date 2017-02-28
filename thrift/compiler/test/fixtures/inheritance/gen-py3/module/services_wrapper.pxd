#
# Autogenerated by Thrift
#
# DO NOT EDIT UNLESS YOU ARE SURE THAT YOU KNOW WHAT YOU ARE DOING
#  @generated
#

from cpython.ref cimport PyObject
from libcpp.memory cimport shared_ptr
from thrift.py3.server cimport cServerInterface



cdef extern from "src/gen-cpp2/MyRoot.h" namespace "cpp2":
    cdef cppclass cMyRootSvAsyncIf "cpp2::MyRootSvAsyncIf":
      pass

    cdef cppclass cMyRootSvIf "cpp2::MyRootSvIf"(
            cMyRootSvAsyncIf,
            cServerInterface):
        pass

cdef extern from "src/gen-cpp2/MyNode.h" namespace "cpp2":
    cdef cppclass cMyNodeSvAsyncIf "cpp2::MyNodeSvAsyncIf":
      pass

    cdef cppclass cMyNodeSvIf "cpp2::MyNodeSvIf"(
            cMyNodeSvAsyncIf,
            module.services_wrapper.cMyRootSvIf,
            cServerInterface):
        pass

cdef extern from "src/gen-cpp2/MyLeaf.h" namespace "cpp2":
    cdef cppclass cMyLeafSvAsyncIf "cpp2::MyLeafSvAsyncIf":
      pass

    cdef cppclass cMyLeafSvIf "cpp2::MyLeafSvIf"(
            cMyLeafSvAsyncIf,
            module.services_wrapper.cMyNodeSvIf,
            cServerInterface):
        pass



cdef extern from "src/gen-py3/module/services_wrapper.h" namespace "cpp2":
    cdef cppclass cMyRootWrapper "cpp2::MyRootWrapper"(
        cMyRootSvIf
    ):
        pass

    shared_ptr[cServerInterface] cMyRootInterface "cpp2::MyRootInterface"(PyObject *if_object)
    cdef cppclass cMyNodeWrapper "cpp2::MyNodeWrapper"(
        cMyNodeSvIf,
        module.services_wrapper.cMyRootWrapper
    ):
        pass

    shared_ptr[cServerInterface] cMyNodeInterface "cpp2::MyNodeInterface"(PyObject *if_object)
    cdef cppclass cMyLeafWrapper "cpp2::MyLeafWrapper"(
        cMyLeafSvIf,
        module.services_wrapper.cMyNodeWrapper
    ):
        pass

    shared_ptr[cServerInterface] cMyLeafInterface "cpp2::MyLeafInterface"(PyObject *if_object)
