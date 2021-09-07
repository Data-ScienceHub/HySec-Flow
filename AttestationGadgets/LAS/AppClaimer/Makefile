include ../buildenv.mk

TARGET = appclaimer

ifneq ($(SGX_MODE), HW)
	URTS_LIB_NAME := sgx_urts_sim
	UAE_SERVICE_LIB := sgx_uae_service_sim
else
	URTS_LIB_NAME := sgx_urts
	UAE_SERVICE_LIB := sgx_uae_service
endif

INC:=-I$(SGX_SDK)/include -I../Include
LIB := -l$(URTS_LIB_NAME) -l$(UAE_SERVICE_LIB) -L$(SGX_SDK)/lib64 -lpthread

CXXFLAGS += $(INC) $(LIB)
CFLAGS += $(INC) $(LIB) 

ifeq ($(SGX_DEBUG), 1)
        CXXFLAGS += -DDEBUG -UNDEBUG -UEDEBUG
        CFLAGS += -DDEBUG -UNDEBUG -UEDEBUG
else ifeq ($(SGX_PRERELEASE), 1)
        CXXFLAGS += -DEDEBUG -DNDEBUG -UDEBUG
        CFLAGS += -DEDEBUG -DNDEBUG -UDEBUG
else
        CXXFLAGS += -DNDEBUG -UEDEBUG -UDEBUG
        CFLAGS += -DNDEBUG -UEDEBUG -UDEBUG
endif

SRC_CPP=$(wildcard *.cpp)
SRC_CPP+= $(wildcard ../util/*.cpp)
SRC_C= $(wildcard ../util/*.c)

SRC_OBJ += $(SRC_CPP:.cpp=.o)
SRC_OBJ += $(SRC_C:.c=.o)

all: $(TARGET)

App.o: App.cpp
	@$(CXX) $(CXXFLAGS) -c $< -o $@
	@echo "CXX  <=  $<"

EnclaveClaimer_u.o: EnclaveClaimer_u.c
	@$(CC) $(CFLAGS) -I../Include -c $< -o $@
	@echo "CC  <=  $<"

EnclaveClaimer_u.c: 
	@$(SGX_EDGER8R) --untrusted ../EnclaveClaimer/EnclaveClaimer.edl --search-path $(SGX_SDK)/include

$(TARGET): EnclaveClaimer_u.o $(SRC_OBJ)
	@$(CXX) $^ $(CXXFLAGS) -o $@
	@echo "GEN  =>  $@"
	@mv $@ $(TOPDIR)/$(OUTDIR)/

clean:
	@$(RM) $(TARGET) *.o *_u.c *_u.h
