SUFFIXES += .d
NODEPS := clean distclean prepare
DEBUG := 0
STATS := 0
MAST := 0
RAVE := 0

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
MAST_CHOOSER := mast_chooser
UNIFORM_CHOOSER := uniform_chooser
MOVES_CONTAINER := moves_container
MOVES_CONTAINER_SPLIT := moves_container_split
MOVES_CONTAINER_CONTEXT := moves_container_context
MOVES_TREE := moves_tree
ACTIONS_ARRAYS := actions_arrays
RAVE_TREE := rave_tree
ORTHODOX_SIMULATOR := orthodox_simulator
SEMISPLIT_SIMULATOR := semisplit_simulator
SEMISPLIT_COMMON := semisplit_mcts_common
SEMISPLIT_MCTS := semisplit_mcts
ROLLUP_MCTS := rollup_mcts

C := g++
ifeq (0, $(DEBUG))
ifeq (0, $(RELEASE))
COMMON_CFLAGS = -Wall -Wextra -Wpedantic -Ofast -march=native -flto -std=c++17 -pthread -DMAST=$(MAST) -DRAVE=$(RAVE) -DSTATS=$(STATS)
else
COMMON_CFLAGS = -Wall -Wextra -Wpedantic -Ofast -march=native -flto -std=c++17 -pthread -s -DNDEBUG -DMAST=$(MAST) -DRAVE=$(RAVE) -DSTATS=$(STATS)
endif
else
COMMON_CFLAGS = -Wall -Wextra -Wpedantic -std=c++17 -pthread -g -DMAST=$(MAST) -DRAVE=$(RAVE) -DSTATS=$(STATS)
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
$(eval $(call PLAYER_KIND_RULES,ORTHODOXMCTS_ORTHODOXSIM,orthodoxMcts_orthodoxSim,$(UNIFORM_CHOOSER) $(MCTS_COMMON) $(ORTHODOX_MCTS) $(ORTHODOX_SIMULATOR) $(COMMON) $(GEN_DIR)))
$(eval $(call PLAYER_KIND_RULES,ORTHODOXMCTS_ORTHODOXSIM_RAVE,orthodoxMcts_orthodoxSim_rave,$(UNIFORM_CHOOSER) $(RAVE_TREE) $(MCTS_COMMON) $(ORTHODOX_MCTS) $(ORTHODOX_SIMULATOR) $(COMMON) $(GEN_DIR)))
$(eval $(call PLAYER_KIND_RULES,ORTHODOXMCTS_ORTHODOXSIM_MAST,orthodoxMcts_orthodoxSim_mast,$(MAST_CHOOSER) $(MOVES_TREE) $(MOVES_CONTAINER) $(MCTS_COMMON) $(ORTHODOX_MCTS) $(ORTHODOX_SIMULATOR) $(COMMON) $(GEN_DIR)))
$(eval $(call PLAYER_KIND_RULES,ORTHODOXMCTS_ORTHODOXSIM_MASTSPLIT,orthodoxMcts_orthodoxSim_mastsplit,$(MAST_CHOOSER) $(MOVES_CONTAINER_SPLIT) $(ACTIONS_ARRAYS) $(MCTS_COMMON) $(ORTHODOX_MCTS) $(ORTHODOX_SIMULATOR) $(COMMON) $(GEN_DIR)))
$(eval $(call PLAYER_KIND_RULES,ORTHODOXMCTS_SEMISPLITSIM,orthodoxMcts_semisplitSim,$(UNIFORM_CHOOSER) $(MCTS_COMMON) $(ORTHODOX_MCTS) $(SEMISPLIT_SIMULATOR) $(COMMON) $(GEN_DIR)))
$(eval $(call PLAYER_KIND_RULES,ORTHODOXMCTS_SEMISPLITSIM_MASTSPLIT,orthodoxMcts_semisplitSim_mastsplit,$(MAST_CHOOSER) $(MOVES_CONTAINER_SPLIT) $(ACTIONS_ARRAYS) $(MCTS_COMMON) $(ORTHODOX_MCTS) $(SEMISPLIT_SIMULATOR) $(COMMON) $(GEN_DIR)))
$(eval $(call PLAYER_KIND_RULES,SEMISPLITMCTS_SEMISPLITSIM,semisplitMcts_semisplitSim,$(UNIFORM_CHOOSER) $(MCTS_COMMON) $(SEMISPLIT_COMMON) $(SEMISPLIT_MCTS) $(SEMISPLIT_SIMULATOR) $(COMMON) $(GEN_DIR)))
$(eval $(call PLAYER_KIND_RULES,SEMISPLITMCTS_SEMISPLITSIM_MAST,semisplitMcts_semisplitSim_mast,$(MAST_CHOOSER) $(MOVES_CONTAINER) $(MOVES_TREE) $(MCTS_COMMON) $(SEMISPLIT_COMMON) $(SEMISPLIT_MCTS) $(SEMISPLIT_SIMULATOR) $(COMMON) $(GEN_DIR)))
$(eval $(call PLAYER_KIND_RULES,SEMISPLITMCTS_SEMISPLITSIM_MASTSPLIT,semisplitMcts_semisplitSim_mastsplit,$(MAST_CHOOSER) $(MOVES_CONTAINER_SPLIT) $(ACTIONS_ARRAYS) $(MCTS_COMMON) $(SEMISPLIT_COMMON) $(SEMISPLIT_MCTS) $(SEMISPLIT_SIMULATOR) $(COMMON) $(GEN_DIR)))
$(eval $(call PLAYER_KIND_RULES,SEMISPLITMCTS_SEMISPLITSIM_MASTCONTEXT,semisplitMcts_semisplitSim_mastcontext,$(MAST_CHOOSER) $(MOVES_TREE) $(ACTIONS_ARRAYS) $(MOVES_CONTAINER_CONTEXT) $(MCTS_COMMON) $(SEMISPLIT_COMMON) $(SEMISPLIT_MCTS) $(SEMISPLIT_SIMULATOR) $(COMMON) $(GEN_DIR)))
$(eval $(call PLAYER_KIND_RULES,SEMISPLITMCTS_SEMISPLITSIM_RAVE,semisplitMcts_semisplitSim_rave,$(UNIFORM_CHOOSER) $(RAVE_TREE) $(MCTS_COMMON) $(SEMISPLIT_COMMON) $(SEMISPLIT_MCTS) $(SEMISPLIT_SIMULATOR) $(COMMON) $(GEN_DIR)))
$(eval $(call PLAYER_KIND_RULES,SEMISPLITMCTS_SEMISPLITSIM_RAVECONTEXT,semisplitMcts_semisplitSim_ravecontext,$(UNIFORM_CHOOSER) $(RAVE_TREE) $(MCTS_COMMON) $(SEMISPLIT_COMMON) $(SEMISPLIT_MCTS) $(SEMISPLIT_SIMULATOR) $(COMMON) $(GEN_DIR)))
$(eval $(call PLAYER_KIND_RULES,SEMISPLITMCTS_SEMISPLITSIM_RAVEMIX,semisplitMcts_semisplitSim_ravemix,$(UNIFORM_CHOOSER) $(RAVE_TREE) $(MCTS_COMMON) $(SEMISPLIT_COMMON) $(SEMISPLIT_MCTS) $(SEMISPLIT_SIMULATOR) $(COMMON) $(GEN_DIR)))
$(eval $(call PLAYER_KIND_RULES,SEMISPLITMCTS_ORTHODOXSIM,semisplitMcts_orthodoxSim,$(UNIFORM_CHOOSER) $(MCTS_COMMON) $(SEMISPLIT_COMMON) $(SEMISPLIT_MCTS) $(ORTHODOX_SIMULATOR) $(COMMON) $(GEN_DIR)))
$(eval $(call PLAYER_KIND_RULES,SEMISPLITMCTS_ORTHODOXSIM_MASTSPLIT,semisplitMcts_orthodoxSim_mastsplit,$(MAST_CHOOSER) $(MOVES_CONTAINER_SPLIT) $(ACTIONS_ARRAYS) $(MCTS_COMMON) $(SEMISPLIT_COMMON) $(SEMISPLIT_MCTS) $(ORTHODOX_SIMULATOR) $(COMMON) $(GEN_DIR)))
$(eval $(call PLAYER_KIND_RULES,SEMISPLITMCTS_ORTHODOXSIM_RAVE,semisplitMcts_orthodoxSim_rave,$(UNIFORM_CHOOSER) $(RAVE_TREE) $(MCTS_COMMON) $(SEMISPLIT_COMMON) $(SEMISPLIT_MCTS) $(ORTHODOX_SIMULATOR) $(COMMON) $(GEN_DIR)))
$(eval $(call PLAYER_KIND_RULES,ROLLUPMCTS_SEMISPLITSIM,rollupMcts_semisplitSim,$(UNIFORM_CHOOSER) $(MCTS_COMMON) $(SEMISPLIT_COMMON) $(ROLLUP_MCTS) $(SEMISPLIT_SIMULATOR) $(COMMON) $(GEN_DIR)))
$(eval $(call PLAYER_KIND_RULES,ROLLUPMCTS_SEMISPLITSIM_MASTCONTEXT,rollupMcts_semisplitSim_mastcontext,$(MAST_CHOOSER) $(MOVES_TREE) $(ACTIONS_ARRAYS) $(MOVES_CONTAINER_CONTEXT) $(MCTS_COMMON) $(SEMISPLIT_COMMON) $(ROLLUP_MCTS) $(SEMISPLIT_SIMULATOR) $(COMMON) $(GEN_DIR)))
$(eval $(call PLAYER_KIND_RULES,ROLLUPMCTS_ORTHODOXSIM,rollupMcts_orthodoxSim,$(UNIFORM_CHOOSER) $(MCTS_COMMON) $(SEMISPLIT_COMMON) $(ROLLUP_MCTS) $(ORTHODOX_SIMULATOR) $(COMMON) $(GEN_DIR)))

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
