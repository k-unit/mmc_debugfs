#include "debugfs.h"

#define MH_DIR "mh"

bool mmc_mh_debugfs_host(struct mmc_host *host, struct dentry *root)
{
	static mmc_host_debugfs_func *host_funcs[] = {
#define DEBUGFS_HOST_ENTRIES
#include "debugfs_entries.h"
	};
	int i;

	if (!host)
		return false;

	root = debugfs_create_dir(MH_DIR, root);
	if (IS_ERR(root)) {
		/* Don't complain -- mh_debugfs just isn't enabled */
		return true;
	}

	for (i = 0; i < ARRAY_SZ(host_funcs); i++) {
		if (!host_funcs[i](host, root))
			return false;
	}

	return true;
}

bool mmc_mh_debugfs_card(struct mmc_card *card, struct dentry *root)
{
	static mmc_card_debugfs_func *card_funcs[] = {
#define DEBUGFS_CARD_ENTRIES
#include "debugfs_entries.h"
	};
	int i;

	if (!card)
		return false;

	root = debugfs_create_dir(MH_DIR, root);
	if (IS_ERR(root)) {
		/* Don't complain -- mh_debugfs just isn't enabled */
		return true;
	}

	for (i = 0; i < ARRAY_SZ(card_funcs); i++) {
		if (!card_funcs[i](card, root))
			return false;
	}

	return true;
}

