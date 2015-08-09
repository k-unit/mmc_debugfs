#if defined(DEBUGFS_DECLARATIONS)
#define DEBUGFS_HOST(func) \
	bool func(struct mmc_host *host, struct dentry *root);
#define DEBUGFS_CARD(func) \
	bool func(struct mmc_card *card, struct dentry *root);
#elif defined(DEBUGFS_HOST_ENTRIES)
#define DEBUGFS_HOST(func) func,
#define DEBUGFS_CARD(func)
#elif defined(DEBUGFS_CARD_ENTRIES)
#define DEBUGFS_HOST(func)
#define DEBUGFS_CARD(func) func,
#endif

/* add debugfs entries here */
DEBUGFS_HOST(mmc_jira_template_debugfs_host)
DEBUGFS_CARD(mmc_jira_template_debugfs_card)
DEBUGFS_CARD(mmc_dev_tree_debugfs)

#undef DEBUGFS_DECLARATIONS
#undef DEBUGFS_HOST_ENTRIES
#undef DEBUGFS_CARD_ENTRIES
#undef DEBUGFS_HOST
#undef DEBUGFS_CARD

