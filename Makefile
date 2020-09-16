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
# MCTS
MCTS_COMMON := mcts_common
SEMISPLIT_MCTS_COMMON := semisplit_mcts_common $(MCTS_COMMON)
ORTHODOX_MCTS := orthodox_mcts $(MCTS_COMMON)
SEMISPLIT_MCTS := semisplit_mcts $(SEMISPLIT_MCTS_COMMON)
ROLLUP_MCTS := rollup_mcts $(SEMISPLIT_MCTS_COMMON)
# MAST
MOVES_TREE := moves_tree
ACTIONS_ARRAYS := actions_arrays
MAST_CHOOSER := mast_chooser
MAST_ORTHODOX := moves_container $(MAST_CHOOSER) $(MOVES_TREE)
MAST_SPLIT := moves_container_split $(MAST_CHOOSER) $(ACTIONS_ARRAYS)
MAST_MIX := moves_container_mix $(MAST_CHOOSER) $(MOVES_TREE) $(ACTIONS_ARRAYS)
MAST_CONTEXT := $(MAST_ORTHODOX)
# RAVE
TGRAVE := rave_tree
RAVE_CONTEXT := $(TGRAVE)
RAVE_MIX := $(TGRAVE)
# SIMULATORS
ORTHODOX_SIMULATOR := orthodox_simulator
SEMISPLIT_SIMULATOR := semisplit_simulator
# OTHERS
UNIFORM_CHOOSER := uniform_chooser

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
ifeq ($(MAKECMDGOALS), $(2))
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
endif
endef

# ORTHODOX-TREE PLAYERS
$(eval $(call PLAYER_KIND_RULES,ORTHODOX_ORTHODOX,orthodox_orthodox,\
			$(ORTHODOX_MCTS) $(ORTHODOX_SIMULATOR) $(UNIFORM_CHOOSER) $(COMMON) $(GEN_DIR)))

$(eval $(call PLAYER_KIND_RULES,ORTHODOX_ORTHODOX_MAST,orthodox_orthodox_mast,\
			$(ORTHODOX_MCTS) $(ORTHODOX_SIMULATOR) $(MAST_ORTHODOX) $(COMMON) $(GEN_DIR)))

$(eval $(call PLAYER_KIND_RULES,ORTHODOX_ORTHODOX_MASTSPLIT,orthodox_orthodox_mastsplit,\
			$(ORTHODOX_MCTS) $(ORTHODOX_SIMULATOR) $(MAST_SPLIT) $(COMMON) $(GEN_DIR)))

$(eval $(call PLAYER_KIND_RULES,ORTHODOX_ORTHODOX_RAVE,orthodox_orthodox_tgrave,\
			$(ORTHODOX_MCTS) $(ORTHODOX_SIMULATOR) $(UNIFORM_CHOOSER) $(TGRAVE) $(COMMON) $(GEN_DIR)))

$(eval $(call PLAYER_KIND_RULES,ORTHODOX_ORTHODOX_MAST_RAVE,orthodox_orthodox_mast_tgrave,\
			$(ORTHODOX_MCTS) $(ORTHODOX_SIMULATOR) $(MAST_ORTHODOX) $(TGRAVE) $(COMMON) $(GEN_DIR)))

$(eval $(call PLAYER_KIND_RULES,ORTHODOX_SEMISPLIT,orthodox_semisplit,\
			$(ORTHODOX_MCTS) $(SEMISPLIT_SIMULATOR) $(UNIFORM_CHOOSER) $(COMMON) $(GEN_DIR)))

$(eval $(call PLAYER_KIND_RULES,ORTHODOX_SEMISPLIT_MASTSPLIT,orthodox_semisplit_mastsplit,\
			$(ORTHODOX_MCTS) $(SEMISPLIT_SIMULATOR) $(MAST_SPLIT) $(COMMON) $(GEN_DIR)))

$(eval $(call PLAYER_KIND_RULES,ORTHODOX_SEMISPLIT_MASTSPLIT_RAVE,orthodox_semisplit_mastsplit_tgrave,\
			$(ORTHODOX_MCTS) $(SEMISPLIT_SIMULATOR) $(MAST_SPLIT) $(TGRAVE) $(COMMON) $(GEN_DIR)))

# SEMISPLIT-TREE PLAYERS
$(eval $(call PLAYER_KIND_RULES,SEMISPLIT_SEMISPLIT,semisplit_semisplit,\
			$(SEMISPLIT_MCTS) $(SEMISPLIT_SIMULATOR) $(UNIFORM_CHOOSER) $(COMMON) $(GEN_DIR)))

$(eval $(call PLAYER_KIND_RULES,SEMISPLIT_SEMISPLIT_MAST,semisplit_semisplit_mast,\
			$(SEMISPLIT_MCTS) $(SEMISPLIT_SIMULATOR) $(MAST_ORTHODOX) $(COMMON) $(GEN_DIR)))

$(eval $(call PLAYER_KIND_RULES,SEMISPLIT_SEMISPLIT_MASTSPLIT,semisplit_semisplit_mastsplit,\
			$(SEMISPLIT_MCTS) $(SEMISPLIT_SIMULATOR) $(MAST_SPLIT) $(COMMON) $(GEN_DIR)))

$(eval $(call PLAYER_KIND_RULES,SEMISPLIT_SEMISPLIT_MASTCONTEXT,semisplit_semisplit_mastcontext,\
			$(SEMISPLIT_MCTS) $(SEMISPLIT_SIMULATOR) $(MAST_CONTEXT) $(COMMON) $(GEN_DIR)))

$(eval $(call PLAYER_KIND_RULES,SEMISPLIT_SEMISPLIT_MASTMIX,semisplit_semisplit_mastmix,\
			$(SEMISPLIT_MCTS) $(SEMISPLIT_SIMULATOR) $(MAST_MIX) $(COMMON) $(GEN_DIR)))

$(eval $(call PLAYER_KIND_RULES,SEMISPLIT_SEMISPLIT_RAVE,semisplit_semisplit_tgrave,\
			$(SEMISPLIT_MCTS) $(SEMISPLIT_SIMULATOR) $(UNIFORM_CHOOSER) $(TGRAVE) $(COMMON) $(GEN_DIR)))

$(eval $(call PLAYER_KIND_RULES,SEMISPLIT_SEMISPLIT_RAVECONTEXT,semisplit_semisplit_ravecontext,\
			$(SEMISPLIT_MCTS) $(SEMISPLIT_SIMULATOR) $(UNIFORM_CHOOSER) $(RAVE_CONTEXT) $(COMMON) $(GEN_DIR)))

$(eval $(call PLAYER_KIND_RULES,SEMISPLIT_SEMISPLIT_RAVEMIX,semisplit_semisplit_ravemix,\
			$(SEMISPLIT_MCTS) $(SEMISPLIT_SIMULATOR) $(UNIFORM_CHOOSER) $(RAVE_MIX) $(COMMON) $(GEN_DIR)))

$(eval $(call PLAYER_KIND_RULES,SEMISPLIT_SEMISPLIT_MAST_RAVE,semisplit_semisplit_mast_tgrave,\
			$(SEMISPLIT_MCTS) $(SEMISPLIT_SIMULATOR) $(MAST_ORTHODOX) $(TGRAVE) $(COMMON) $(GEN_DIR)))

$(eval $(call PLAYER_KIND_RULES,SEMISPLIT_SEMISPLIT_MAST_RAVECONTEXT,semisplit_semisplit_mast_ravecontext,\
			$(SEMISPLIT_MCTS) $(SEMISPLIT_SIMULATOR) $(MAST_ORTHODOX) $(RAVE_CONTEXT) $(COMMON) $(GEN_DIR)))

$(eval $(call PLAYER_KIND_RULES,SEMISPLIT_SEMISPLIT_MASTSPLIT_RAVE,semisplit_semisplit_mastsplit_tgrave,\
			$(SEMISPLIT_MCTS) $(SEMISPLIT_SIMULATOR) $(MAST_SPLIT) $(TGRAVE) $(COMMON) $(GEN_DIR)))

$(eval $(call PLAYER_KIND_RULES,SEMISPLIT_SEMISPLIT_MASTSPLIT_RAVECONTEXT,semisplit_semisplit_mastsplit_ravecontext,\
			$(SEMISPLIT_MCTS) $(SEMISPLIT_SIMULATOR) $(MAST_SPLIT) $(RAVE_CONTEXT) $(COMMON) $(GEN_DIR)))

$(eval $(call PLAYER_KIND_RULES,SEMISPLIT_SEMISPLIT_MASTCONTEXT_RAVECONTEXT,semisplit_semisplit_mastcontext_ravecontext,\
			$(SEMISPLIT_MCTS) $(SEMISPLIT_SIMULATOR) $(MAST_CONTEXT) $(RAVE_CONTEXT) $(COMMON) $(GEN_DIR)))

$(eval $(call PLAYER_KIND_RULES,SEMISPLIT_SEMISPLIT_MASTMIX_RAVECONTEXT,semisplit_semisplit_mastmix_ravecontext,\
			$(SEMISPLIT_MCTS) $(SEMISPLIT_SIMULATOR) $(MAST_MIX) $(RAVE_CONTEXT) $(COMMON) $(GEN_DIR)))

$(eval $(call PLAYER_KIND_RULES,SEMISPLIT_ORTHODOX,semisplit_orthodox,\
			$(SEMISPLIT_MCTS) $(ORTHODOX_SIMULATOR) $(UNIFORM_CHOOSER) $(COMMON) $(GEN_DIR)))

$(eval $(call PLAYER_KIND_RULES,SEMISPLIT_ORTHODOX_MASTSPLIT,semisplit_orthodox_mastsplit,\
			$(SEMISPLIT_MCTS) $(ORTHODOX_SIMULATOR) $(MAST_SPLIT) $(COMMON) $(GEN_DIR)))

# ROLLUP-TREE PLAYERS
$(eval $(call PLAYER_KIND_RULES,ROLLUP_SEMISPLIT,rollup_semisplit,\
			$(ROLLUP_MCTS) $(SEMISPLIT_SIMULATOR) $(UNIFORM_CHOOSER) $(COMMON) $(GEN_DIR)))

$(eval $(call PLAYER_KIND_RULES,ROLLUP_SEMISPLIT_MASTCONTEXT,rollup_semisplit_mastcontext,\
			$(ROLLUP_MCTS) $(SEMISPLIT_SIMULATOR) $(MAST_CONTEXT) $(COMMON) $(GEN_DIR)))

$(eval $(call PLAYER_KIND_RULES,ROLLUP_SEMISPLIT_MASTMIX,rollup_semisplit_mastmix,\
			$(ROLLUP_MCTS) $(SEMISPLIT_SIMULATOR) $(MAST_MIX) $(COMMON) $(GEN_DIR)))

$(eval $(call PLAYER_KIND_RULES,ROLLUP_ORTHODOX,rollup_orthodox,\
			$(ROLLUP_MCTS) $(ORTHODOX_SIMULATOR) $(UNIFORM_CHOOSER) $(COMMON) $(GEN_DIR)))

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
