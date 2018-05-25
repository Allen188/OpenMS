from String cimport *

from libcpp cimport bool

cdef extern from "<OpenMS/FORMAT/ExperimentalDesignFile.h>" namespace "OpenMS":
    cdef cppclass ExperimentalDesignFile "OpenMS::ExperimentalDesignFile":
        ExperimentalDesignFile() nogil except +
                
# COMMENT: wrap static methods
cdef extern from "<OpenMS/FORMAT/ExperimentalDesignFile.h>" namespace "OpenMS::ExperimentalDesignFile":
    ExperimentalDesignFile load(const String &tsv_file, bool) nogil except + #wrap-attach:ExperimentalDesignFile

