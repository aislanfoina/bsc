#######################################################################
#  Splotch V4.1                                                       #
#######################################################################

#--------------------------------------- Basic operation mode of code
#OPT	+=  -DINTERPOLATE
#OPT	+=  -DHIGH_ORDER_INTERPOLATION


#--------------------------------------- Switch on MPI
#OPT	+=  -DUSE_MPI
#OPT	+=  -DUSE_MPIIO

#--------------------------------------- Switch on HDF5

#OPT     +=  -DHDF5
#OPT     +=  -DH5_USE_16_API

#--------------------------------------- Visual Studio Option
#OPT	+=  -DVS

#--------------------------------------- CUDA options
#OPT     +=  -DCUDA
OPT     +=  -DNO_WIN_THREAD

#--------------------------------------- Select target Computer

#SYSTYPE="SP6"
SYSTYPE="GP"
#SYSTYPE="PLX"

ifeq (HDF5,$(findstring HDF5,$(OPT)))
HDF5_HOME = /cineca/prod/libraries/hdf5/1.8.4_ser/xl--10.1
LIB_HDF5  = -L$(HDF5_HOME)/lib -lhdf5 -L/cineca/prod/libraries/zlib/1.2.3/xl--10.1/lib/ -lz -L/cineca/prod/libraries/szlib/2.1/xl--10.1/lib/ -lsz
HDF5_INCL = -I$(HDF5_HOME)/include
endif

ifeq (USE_MPI,$(findstring USE_MPI,$(OPT)))
CC       = mpic++        # sets the C-compiler (default)
else
CC       = g++        # sets the C-compiler (default)
endif
OMP      = -fopenmp

OPTIMIZE = -std=c++98 -pedantic -Wno-long-long -Wfatal-errors -Wextra -Wall -Wstrict-aliasing=2 -Wundef -Wshadow -Wwrite-strings -Wredundant-decls -Woverloaded-virtual -Wcast-qual -Wcast-align -Wpointer-arith -Wold-style-cast -O2 -g    # optimization and warning flags (default)
SUP_INCL = -I. -Icxxsupport -Impiio-1.0/include/

ifeq ($(SYSTYPE),"SP6")
ifeq (USE_MPI,$(findstring USE_MPI,$(OPT)))
CC       =  mpCC_r
else
CC       =  xlc++        
endif
OPTIMIZE =  -q64 -O3 -qarch=auto -qtune=auto -qinline
LIB_OPT	 =  -bstackpsize:64k -bdatapsize:64k -btextpsize:64k
OMP =
endif

ifeq ($(SYSTYPE),"GP")
ifeq (USE_MPI,$(findstring USE_MPI,$(OPT)))
CC       =  nvcc -g
else
CC       =  sscxx -g
endif
OPTIMIZE = -O2 
LIB_OPT  = 
OMP =  
#-Xcompiler -openmp
SUP_INCL += -I/opt/amd64/cuda/include -lcuda 
#-I$(CUDA_HOME)/sdk/common/inc -I$(CUDA_HOME)/sdk/C/common/inc # -I$(CUDA_HOME)/include  -Icuda
endif

ifeq ($(SYSTYPE),"PLX")
ifeq (USE_MPI,$(findstring USE_MPI,$(OPT)))
CC       =  nvcc -g
else
CC       =  nvcc -g
endif
OPTIMIZE = -O2 -DDEBUG
LIB_OPT  = -Xlinker -L$(NVCC_HOME)/lib
OMP =
SUP_INCL += -I$(CUDASDK_HOME)/common/inc -I$(NVCC_HOME)/include -Icuda
endif

#--------------------------------------- Here we go

OPTIONS = $(OPTIMIZE) $(OPT)

EXEC   = Splotch4.1$(SYSTYPE)

OBJS = kernel/transform.o

OBJS +=	cxxsupport/error_handling.o \
        reader/mesh_reader.o \
	cxxsupport/mpi_support.o cxxsupport/cxxutils.o reader/gadget_reader.o \
	reader/millenium_reader.o reader/bin_reader.o reader/bin_reader_mpi.o \
	writer/write_tga.o splotch/splotchutils.o splotch/splotch.o splotch/scenemaker.o cxxsupport/walltimer.o

ifeq (CUDA,$(findstring CUDA,$(OPT)))
OBJS += cuda/splotch.o cuda/CuPolicy.o cuda/splotch_cuda2.o
endif

ifeq (HDF5,$(findstring HDF5,$(OPT)))
OBJS += reader/hdf5_reader.o 
endif
ifeq (USE_MPIIO,$(findstring USE_MPIIO,$(OPT)))
LIB_MPIIO = -Lmpiio-1.0/lib -lpartition
endif

INCL   = */*.h Makefile

CPPFLAGS = $(OPTIONS) $(SUP_INCL) $(HDF5_INCL) $(OMP)

CUFLAGS = $(OPTIONS) $(SUP_INCL)

LIBS   = $(LIB_OPT) $(OMP)

.SUFFIXES: .o .cc .cxx .cpp .cu

.cc.o:
	$(CC) -c $(CPPFLAGS) -o "$@" "$<"

.cxx.o:
	$(CC) -c $(CPPFLAGS) -o "$@" "$<"

.cpp.o:
	$(CC) -c $(CPPFLAGS) -o "$@" "$<"

.cu.o:
	$(CC) -c $(CUFLAGS) -o "$@" "$<"

$(EXEC): $(OBJS)
	$(CC) $(OPTIONS) $(OBJS) $(LIBS) $(RLIBS) -o $(EXEC) $(LIB_MPIIO) $(LIB_HDF5)

$(OBJS): $(INCL)

clean:
	rm -f $(OBJS) $(EXEC)

