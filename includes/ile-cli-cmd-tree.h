/***************************************************************************\
#                            ile-cli-cmd-tree.h                             #
\***************************************************************************/

#ifndef ILE_CLI_CMD_TREE_H
#define ILE_CLI_CMD_TREE_H

#include <ile-cli-config.h>

typedef struct ile_cmd_candidates_attr {
	unsigned int offset;
	unsigned int candidate_counter;
	unsigned int common_length;
} ile_cmd_candidates_attr;

int ile_cli_cmd_tree_branch_exe(const int argc, char **const argv, unsigned int *node_counter);
void ile_cli_cmd_root_grp_info_show(void);
void ile_cli_cmd_candidate_lookup(const int argc, char **const argv, int whitespace);
const char* ile_cli_cmd_candidate_completion(const int argc, char **const argv, struct ile_cmd_candidates_attr *candidates_attr, int whitespace);

const char *ile_cli_cmd_node_info_get(node_t cmd_node);
void ile_cmd_node_allocator_override(struct ile_cli_operations *op);

#endif /* CMD_TREE */
