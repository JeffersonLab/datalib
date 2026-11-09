set(DATALIB_CONFIG "/Users/admin/Downloads/macos-datalib-reorg/evidence/chroma-storage-20261109-001/install/datalib-filedb/bin/datalib-config")
     foreach(flag cxxflags ldflags libs storage-cxxflags storage-ldflags storage-libs)
       execute_process(COMMAND "${DATALIB_CONFIG}" "--${flag}"
         RESULT_VARIABLE datalib_status OUTPUT_VARIABLE datalib_${flag}
         OUTPUT_STRIP_TRAILING_WHITESPACE)
       if(NOT datalib_status EQUAL 0)
         message(FATAL_ERROR "datalib-config --${flag} failed")
       endif()
       separate_arguments(datalib_${flag} NATIVE_COMMAND "${datalib_${flag}}")
     endforeach()

message(STATUS "Storage compile/link queries passed")
