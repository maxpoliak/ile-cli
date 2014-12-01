/***************************************************************************\
#                            ile-cli-cmd-tree.c                             #
\***************************************************************************/

#include <ile-cli-api.h>
#include <ile-cli-cmd-tree.h>
#include <unistd.h>
#include <string.h>

#ifndef ILE_CLI_CMD_DEFAULT_ACCESS_LVL
#define ILE_CLI_CMD_DEFAULT_ACCESS_LVL	3
#endif

#ifndef ILE_CLI_CMD_NODE_NUM_LIMIT
#define ILE_CLI_CMD_NODE_NUM_LIMIT		16
#endif

#ifndef CMD_TREE_NODES_LIMIT_VERTICAL
#define CMD_TREE_NODES_LIMIT_VERTICAL	32
#endif

#define CMD_NAME_MAX_LENGTH(name1, name2) \
	(strlen((name1)) > strlen((name2)) ? strlen((name1)) : strlen((name2)))

struct ile_command_node {
	unsigned char flags;
	struct ile_command_node *child;
	struct ile_command_node *next;
	const char *name;
	const char *info;
	int (*exec)(node_t self, const int argc, char **const argv);
	unsigned char access_lvl;
};

static struct ile_command_node cmd_node[ILE_CLI_CMD_NODE_NUM_LIMIT];
static struct ile_command_node *root_node;

static void* (*override_node_allocator)(size_t size) = NULL;

/*
 * ile_cmd_node_new()
 * @op
 */
void ile_cmd_node_allocator_override(struct ile_cli_operations *op)
{
	override_node_allocator = op->node_allocator;
}

/*
 * ile_cmd_node_new()
 * return : a pointer to a new free command node pointer of NULL if
 *          there are no free nodes
 */
static struct ile_command_node* ile_cmd_node_new(void)
{
	static unsigned int entry_point = 0;

	if (override_node_allocator)
		return override_node_allocator(sizeof(struct ile_command_node));
	else if (entry_point < ILE_CLI_CMD_NODE_NUM_LIMIT)
		return &cmd_node[entry_point++];
	else
		return NULL;
}

/*
 * ile_command_root_node_set()
 * @root : a pointer to root node
 */
node_t ile_command_root_node_set(node_t root)
{
	struct ile_command_node *node = (struct ile_command_node*) root;
	if (!ILE_CMD_FLAG_TST(IS_ROOT, node))
		return NULL;
	root_node = node;
	return root;
}

/*
 * ile_command_root_node_get()
 * return : a pointer to root node
 */
node_t ile_command_root_node_get(void)
{
	return (node_t)root_node;
}

/*
 * ile_cli_cmd_node_info_get()
 * @cmd_node : command node descriptor.
 * return : information string about cmd_node
 */
const char *ile_cli_cmd_node_info_get(node_t cmd_node)
{
	const struct ile_command_node* node = (const struct ile_command_node*) cmd_node;
	return node->info;
}

/*
 * ile_cli_cmd_root_node_add()
 * @parent      : descriptor of a parent node
 * @banner_name : name of the additional cli banner
 * @info        : help info
 * return : descriptor of a new root node
 */
node_t ile_cli_cmd_root_node_add(node_t parent, const char* banner_name, const char* info)
{
	struct ile_command_node* root = ile_cmd_node_new();
	if (!root)
		return NULL;
	root->name = banner_name;
	root->info = info;
	root->next = (struct ile_command_node*)parent;
	ILE_CMD_FLAG_SET(IS_ROOT, root);
	cli_info_print(ILE_CLI_WHITE_COLOUR, "Add <%s> root\n", banner_name);
	return (node_t)root;
}

/*
 * ile_cli_cmd_raw_node_add()
 * @parent     : descriptor of a parent node
 * @name       : node name
 * @info       : help info
 * @exec       : executable function
 * @access_lvl : command access level
 * @flags      : flags
 * return : descriptor of a new node
 */
static node_t ile_cli_cmd_raw_node_add(
	node_t parent,
	const char* name,
	const char* info,
	int (*exec)(node_t self, const int argc, char **const argv),
	const unsigned char access_lvl,
	const unsigned char flags)
{
	register unsigned int i;
	struct ile_command_node **node;
	if (!parent)
		return NULL;
	for (i = 0, node = &(((struct ile_command_node*)parent)->child);
	     (*node);
		 node = &((*node)->next), ++i) {
		if (i >= CMD_TREE_NODES_LIMIT_VERTICAL)
			return NULL;
	}
	*node = ile_cmd_node_new();
	if (!(*node))
		return NULL;
	cli_info_print(ILE_CLI_WHITE_COLOUR, "Add <%s>\n", name);
	(*node)->name = name;
	(*node)->info = info;
	(*node)->exec = exec;
	(*node)->access_lvl = access_lvl;
	(*node)->flags = flags;
	return *node;
}
/*
 * ile_cli_cmd_exec_node_add()
 */
node_t ile_cli_cmd_exec_node_add(
	node_t parent,
	const char* name,
	const char* info,
	int (*exec)(node_t self, const int argc, char **const argv))
{
	return ile_cli_cmd_raw_node_add(parent, name, info, exec, ILE_CLI_CMD_DEFAULT_ACCESS_LVL, 0);
}
/*
 * ile_cli_cmd_exec_node_add()
 */
node_t ile_cli_cmd_exec_node_flags_add(
	node_t parent,
	const char* name,
	const char* info,
	int (*exec)(node_t self, const int argc, char **const argv),
	const unsigned char flags)
{
	return ile_cli_cmd_raw_node_add(parent, name, info, exec, ILE_CLI_CMD_DEFAULT_ACCESS_LVL, flags);
}
/*
 * ile_cli_cmd_node_add()
 */
node_t ile_cli_cmd_node_add(node_t parent, const char* name, const char* info)
{
	return ile_cli_cmd_raw_node_add(parent, name, info, NULL, ILE_CLI_CMD_DEFAULT_ACCESS_LVL, 0);
}
/*
 * ile_cli_cmd_node_add()
 */
node_t ile_cli_cmd_node_flags_add(node_t parent, const char* name, const char* info, const unsigned char flags)
{
	return ile_cli_cmd_raw_node_add(parent, name, info, NULL, ILE_CLI_CMD_DEFAULT_ACCESS_LVL, flags);
}

#define PASS_UNCHECKED	0
#define LOOKUP_ALL		1
/*
 * ile_cli_cmd_group_lookup()
 * @node
 * @argument
 * @arg_len
 * @lookup_flag
 * return node
 */
static struct ile_command_node*
ile_cli_cmd_group_lookup(struct ile_command_node *node, const char *argument, const ssize_t arg_len, int lookup_flag)
{
	register unsigned int i;
	if (node && arg_len && argument) {
		/*
		 *    (ilecli) > arg0  <argument>  arg2  argx
		 * parent --> child0 --> node0
		 *                        ...
		 *                       node[x] <-- <node>
		 *                       node[x + 1]
		 *                       node[x + 2] == argument -> return node[x + 2]
		 */
		for(i = 0; node && i < CMD_TREE_NODES_LIMIT_VERTICAL; node = node->next, ++i) {
			if (!strncmp(argument, node->name, arg_len) || (lookup_flag && ILE_CMD_FLAG_TST(UNCHECKED, node)))
				break;
		}
	}
	return node;
}

/*
 * ile_cli_cmd_lookup_tree()
 * @argc        : argument counter
 * @argv        : command line arguments
 * @nodes_cnt   : (returns) the number of nodes in a tree branch
 * @lookup_flag : LOOKUP_ALL or PASS_UNCHECKED
 * return : last command node in the tree that matches the corresponding
 *          argument
 */
static struct ile_command_node*
ile_cli_cmd_lookup_tree(const int argc, char ** const argv, unsigned int *nodes_cnt, int lookup_flag)
{
	register unsigned int i;
	struct ile_command_node *nodeX, *node = root_node;
	if (!node || !node->child)
		return NULL;
	if (!argc || !argv[0])
		return root_node;
	/*
	 *  (ilecli) >  arg0      arg1      arg2
	 *               ==        ==        !=
	 *  [root] --> child
	 *             group0
	 *               |
	 *               V
	 *   <arg0> == node0
	 *             node1 --> child
	 *             node2     group1
	 *              ...        |
	 *             nodeX       V
	 *             <arg1> == node0 --> child
	 *                       node1     group2
	 *                        ...        |
	 *                       nodeX       V
	 *                                 node0
	 *                                  ...
	 *                       <agr2> != nodeX --> return node0[child group1]
	 */
	for (i = 0, nodeX = node->child; nodeX && i < argc; node = nodeX, nodeX = node->child, ++i) {
		nodeX = ile_cli_cmd_group_lookup(nodeX, argv[i], CMD_NAME_MAX_LENGTH(nodeX->name, argv[i]), lookup_flag);
		if (!nodeX)
			break;
	}
	*nodes_cnt = i;
	return node;	
}

/*
 * ile_cli_cmd_node_info_show()
 * @node : command node
 */
static inline
void ile_cli_cmd_node_info_show(const struct ile_command_node *node)
{
	if (node)
		cli_info_print(ILE_CLI_WHITE_COLOUR, "%-28.20s %s\n", node->name, node->info);
}

/*
 * ile_cli_cmd_group_candidates_show()
 * @parent   : command node
 * @argument : command line argument
 * @arg_len  : length of this argument
 * return : candidate node
 */
static void ile_cli_cmd_group_candidates_show(
	struct ile_command_node *node,
	const char *argument,
	const ssize_t arg_len)
{
	register unsigned int i;
	node = ile_cli_cmd_group_lookup(node, argument, arg_len, LOOKUP_ALL);
	for(i = 0; node && i < CMD_TREE_NODES_LIMIT_VERTICAL; ++i) {
		ile_cli_cmd_node_info_show(node);
		node = ile_cli_cmd_group_lookup(node->next, argument, arg_len, LOOKUP_ALL);
	}
}
/*
 * ile_cli_cmd_group_candidates_lookup()
 * @node
 * @argument
 * @arg_len
 * @candidates_attr
 * return candidate node
 */ 
static struct ile_command_node* ile_cli_cmd_group_candidates_lookup(
	struct ile_command_node *node,
	const char *argument,
	const ssize_t arg_len,
	struct ile_cmd_candidates_attr *candidates_attr)
{
	register unsigned int i;
	struct ile_command_node *candidate = ile_cli_cmd_group_lookup(node, argument, arg_len, PASS_UNCHECKED);
	for(i = 0; candidate && i < CMD_TREE_NODES_LIMIT_VERTICAL; candidate = node, ++i) {
		node = ile_cli_cmd_group_lookup(candidate->next, argument, arg_len, PASS_UNCHECKED);
		if (node) {
			if (candidates_attr->common_length > strspn(candidate->name, node->name))
				candidates_attr->common_length = strspn(candidate->name, node->name);
		} else if (!i) {
			candidates_attr->common_length = strlen(candidate->name);
			break; // one candidate
		} else if (arg_len && candidates_attr->common_length > arg_len) {
			break; // several candidates have a common part
		}
	}
	candidates_attr->candidate_counter = i;
	return candidate;
}

const char* srt_exec_cr_label = "<cr>";
const char* srt_exec_cr_help  = "Press Enter to execute commands";

/*
 * ile_cli_cmd_candidates_lookup()
 * @argc   : argument counter
 * @argv   : command line arguments
 * @space  : whitespace flag
 */
void ile_cli_cmd_candidate_lookup(const int argc, char **const argv, int whitespace)
{
	struct ile_command_node *node;
	unsigned int node_counter = 0;

	if (!root_node || argc > ILE_CLI_CMD_NODE_NUM_LIMIT)
		return;
	if (!argc || !argv[0]) {
		ile_cli_cmd_group_candidates_show(root_node->child, NULL, 0);
		return;
	}
	node = ile_cli_cmd_lookup_tree(argc, argv, &node_counter, LOOKUP_ALL);
	if (node) {
		if (node_counter == argc && !whitespace) /* no? */ {
			ile_cli_cmd_group_candidates_show(node, argv[argc - 1], strlen(argv[argc - 1]));
		} else if (node_counter == argc && whitespace) /* node ? */ {
			if (node->exec)
				cli_info_print(ILE_CLI_WHITE_COLOUR, "%-28.20s %s\n", srt_exec_cr_label, srt_exec_cr_help);
			ile_cli_cmd_group_candidates_show(node->child, argv[argc - 1], 0);
		} else if (node_counter == argc - 1 && !whitespace) /* node child? */ {
			ile_cli_cmd_group_candidates_show(node->child, argv[argc - 1], strlen(argv[argc - 1]));
		}
	}
}

/*
 * ile_cli_cmd_candidate_for_completion()
 * @argc            : argument counter
 * @argv            : command line arguments
 * @candidates_attr : attributes of all candidates
 * @whitespace      : whitespace flag
 * return : candidate name
 */
const char* ile_cli_cmd_candidate_completion(
	const int argc,
	char **const argv,
	struct ile_cmd_candidates_attr *candidates_attr,
	int whitespace)
{
	struct ile_command_node *node, *elected_candidate;
	unsigned int node_counter = 0;
	const char *last_argument;
	ssize_t arg_len;

	if (!root_node || argc > ILE_CLI_CMD_NODE_NUM_LIMIT || !argc || !argv[0])
		return NULL;

	last_argument = argv[argc - 1];
	arg_len = strlen(last_argument);
	node = ile_cli_cmd_lookup_tree(argc, argv, &node_counter, PASS_UNCHECKED);
	if (!node)
		return NULL;
	if (node_counter == argc && !whitespace) { /* no? */
		elected_candidate = ile_cli_cmd_group_candidates_lookup(node, last_argument, arg_len, candidates_attr);
	} else if (node_counter == argc && whitespace) { /* node ? */
		elected_candidate = ile_cli_cmd_group_candidates_lookup(node->child, last_argument, 0, candidates_attr);
	} else if (node_counter == argc - 1 && !whitespace) { /* node child? */
		elected_candidate = ile_cli_cmd_group_candidates_lookup(node->child, last_argument, arg_len, candidates_attr);
	} else {
		return NULL;
	}
	if (elected_candidate) {
		candidates_attr->offset = !whitespace ? arg_len : 0;
		return elected_candidate->name;
	} else if (candidates_attr->candidate_counter >= 2) {
		cli_info_print(ILE_CLI_WHITE_COLOUR, "\n");
		if (node_counter == argc && !whitespace) /* no? */
			ile_cli_cmd_group_candidates_show(node, last_argument, arg_len);
		else if (node_counter == argc && whitespace) /* node ? */
			ile_cli_cmd_group_candidates_show(node->child, last_argument, 0);
		else if (node_counter == argc - 1 && !whitespace) /* node child? */
			ile_cli_cmd_group_candidates_show(node->child, last_argument, arg_len);		
	}
	return NULL;
}

/*
 * ile_cli_cmd_tree_branch_exe()
 * @argc         : argument counter
 * @argv         : command line arguments
 * @node_counter : node counter in current branch
 */
int ile_cli_cmd_tree_branch_exe(const unsigned int argc, char **const argv, unsigned int *node_counter)
{
	struct ile_command_node *exec_node = ile_cli_cmd_lookup_tree(argc, argv, node_counter, LOOKUP_ALL);
	if (exec_node->exec && *node_counter == argc) {
		cli_info_print(ILE_CLI_WHITE_COLOUR, "\n");
		return exec_node->exec(exec_node, argc, argv);
	}
	return 1;
}
