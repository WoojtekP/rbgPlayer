SUFFIXES += .d
NODEPS := clean distclean prepare

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

C := g++
COMMON_CFLAGS = -Wall -Wextra -Wpedantic -Ofast -march=native -flto -std=c++17 -pthread -s

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
$(eval $(call PLAYER_KIND_RULES,MCTS,mcts_orthodox,$(MCTS_COMMON) $(ORTHODOX_MCTS) $(COMMON) $(GEN_DIR)))
$(eval $(call PLAYER_KIND_RULES,MAST,mast,$(MCTS_COMMON) $(MAST_COMMON) $(MAST) $(COMMON) $(GEN_DIR)))
$(eval $(call PLAYER_KIND_RULES,MAST_SEMISPLIT,mast_semisplit,$(MCTS_COMMON) $(MAST_COMMON) $(MAST_SEMISPLIT) $(COMMON) $(GEN_DIR)))

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
