CXXFLAGS	+= -I./sources/ak

VPATH += sources/ak

OBJ += $(OBJ_DIR)/ak.o
OBJ += $(OBJ_DIR)/message.o
OBJ += $(OBJ_DIR)/timer.o
OBJ += $(OBJ_DIR)/fsm.o
OBJ += $(OBJ_DIR)/tsm.o
