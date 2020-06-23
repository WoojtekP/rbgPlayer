SUFFIXES += .d
NODEPS := clean distclean prepare
DEBUG := 0

PLAYER_ID := 99999
TARGET := rbgPlayer
COMPILER_DIR := rbg2cpp
COMPILER_BIN_DIR := bin
COMPILER := rbg2cpp
SRC_DIR := src
INC_DIR := inc
OBJ_DIR := obj_$(PLAYER_ID)
BIN_DIR := bin_$(PLAYER_ID)
DEP_DIR := dep_$(PLAYER_ID)
GEN_DIR := gen_$(PLAYER_ID)
COMMON := common
RANDOM := random
SBS := simple_best_select
ORTHODOX_MCTS := orthodox_mcts
MCTS_COMMON := mcts_common
MAST_COMMON := mast_common
MAST := moves_container
MAST_SEMISPLIT := moves_container_semisplit
ORTHODOX_SIMULATOR := orthodox_simulator
SEMISPLIT_SIMULATOR := semisplit_simulator
JOINT_MOVES := joint_moves
TREE_MOVE_JOIN := 0
SIM_MOVE_JOIN := 0

C := g++
ifeq (0, $(DEBUG))
COMMON_CFLAGS = -Wall -Wextra -Wpedantic -Ofast -march=native -flto -std=c++17 -pthread -s -DTREE_MOVE_JOIN=$(TREE_MOVE_JOIN) -DSIM_MOVE_JOIN=$(SIM_MOVE_JOIN)
else
COMMON_CFLAGS = -Wall -Wextra -Wpedantic -std=c++17 -pthread -g
endif

define OBJECT_RULES
$(OBJ_DIR)/$(1)/%.o: $(3)/$(SRC_DIR)/%.cpp $(DEP_DIR)/$(1)/%.d | $(OBJ_DIR)/$(1)
	$(C) $(2) -c $$< -o $$@
$(DEP_DIR)/$(1)/%.d: $(3)/$(SRC_DIR)/%.cpp | $(DEP_DIR)/$(1)
	$(C) $(2) -MM -MT '$$(patsubst $(3)/$(SRC_DIR)/%.cpp,$(OBJ_DIR)/$(1)/%.o,$$<) $$@' $$< -MF $$@
endef

define PLAYER_KIND_RULES
$(1)_DIRS := $(3)
$(1)_INCLUDE := $$(foreach dir,$$($(1)_DIRS),-I$$(wildcard $$(dir)/$(INC_DIR)))
$(1)_CFLAGS := $(COMMON_CFLAGS) $$($(1)_INCLUDE)
$(1)_OBJECTS := $$(foreach dir,$$($(1)_DIRS),$$(patsubst $$(dir)/$(SRC_DIR)/%.cpp, $(OBJ_DIR)/$(2)/%.o, $$(wildcard $$(dir)/$(SRC_DIR)/*.cpp)))
$$(foreach dir,$$($(1)_DIRS),$$(eval $$(call OBJECT_RULES,$(2),$$($(1)_CFLAGS),$$(dir))))
$(1)_DEP := $$(foreach dir,$$($(1)_DIRS),$$(patsubst $$(dir)/$(SRC_DIR)/%.cpp, $(DEP_DIR)/$(2)/%.d, $$(wildcard $$(dir)/$(SRC_DIR)/*.cpp)))
DEPFILES := $(DEPFILES) $$($(1)_DEP)
$(OBJ_DIR)/$(2): | $(OBJ_DIR)
	mkdir -p $(OBJ_DIR)/$(2)
$(DEP_DIR)/$(2): | $(DEP_DIR)
	mkdir -p $(DEP_DIR)/$(2)
$(2): $$($(1)_OBJECTS) | $(BIN_DIR)
	$(C) $$($(1)_CFLAGS) $$($(1)_OBJECTS) -o $(BIN_DIR)/$$@
ifeq (1, $$(words $$(findstring $(MAKECMDGOALS), $(2))))
    -include $$($(1)_DEP)
endif
endef

$(eval $(call PLAYER_KIND_RULES,RANDOM,random,$(RANDOM) $(GEN_DIR)))
$(eval $(call PLAYER_KIND_RULES,SBS,simple_best_select,$(SBS) $(GEN_DIR)))
$(eval $(call PLAYER_KIND_RULES,SEMISPLIT_MCTS_SIM_SEMISPLIT,semisplit_mcts_sim_semisplit,mcts_semisplit mcts_semisplit_tree $(SEMISPLIT_SIMULATOR) $(COMMON) $(GEN_DIR)))
$(eval $(call PLAYER_KIND_RULES,MCTS_SIM_ORTHODOX,mcts_sim_orthodox,$(MCTS_COMMON) $(ORTHODOX_MCTS) $(ORTHODOX_SIMULATOR) $(COMMON) $(GEN_DIR)))
$(eval $(call PLAYER_KIND_RULES,MCTS_SIM_JOINT,mcts_sim_joint,$(MCTS_COMMON) $(ORTHODOX_MCTS) $(COMMON) $(ORTHODOX_SIMULATOR) $(JOINT_MOVES) $(GEN_DIR)))
$(eval $(call PLAYER_KIND_RULES,MCTS_SIM_SEMISPLIT,mcts_sim_semisplit,$(MCTS_COMMON) $(ORTHODOX_MCTS) $(SEMISPLIT_SIMULATOR) $(COMMON) $(GEN_DIR)))
$(eval $(call PLAYER_KIND_RULES,MCTS_JOINT_SIM_ORTHODOX,mcts_joint_sim_orthodox,$(MCTS_COMMON) $(ORTHODOX_MCTS) $(ORTHODOX_SIMULATOR) $(JOINT_MOVES) $(COMMON) $(GEN_DIR)))
$(eval $(call PLAYER_KIND_RULES,MCTS_JOINT_SIM_SEMISPLIT,mcts_joint_sim_semisplit,$(MCTS_COMMON) $(ORTHODOX_MCTS) $(SEMISPLIT_SIMULATOR) $(JOINT_MOVES) $(COMMON) $(GEN_DIR)))
$(eval $(call PLAYER_KIND_RULES,MCTS_JOINT_SIM_JOINT,mcts_joint_sim_joint,$(MCTS_COMMON) $(ORTHODOX_MCTS) $(ORTHODOX_SIMULATOR) $(JOINT_MOVES) $(COMMON) $(GEN_DIR)))
$(eval $(call PLAYER_KIND_RULES,MCTS_MAST_SIM_ORTHODOX,mcts_mast_sim_orthodox,$(MCTS_COMMON) $(MAST_COMMON) $(MAST) $(ORTHODOX_SIMULATOR) $(COMMON) $(GEN_DIR)))
$(eval $(call PLAYER_KIND_RULES,MCTS_MAST_SEMISPLIT_SIM_ORTHODOX,mcts_mast_semisplit_sim_orthodox,$(MCTS_COMMON) $(MAST_COMMON) $(MAST_SEMISPLIT) $(ORTHODOX_SIMULATOR) $(COMMON) $(GEN_DIR)))

$(DEP_DIR):
	mkdir -p $(DEP_DIR)

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

prepare:
	cd $(COMPILER_DIR); make $(COMPILER); cd ..

clean:
	rm -rf obj*
	rm -rf dep*
	rm -rf gen*

distclean: clean
	rm -rf bin*
