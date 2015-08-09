#ifndef _MH_DEBUGFS_H_
#define _MH_DEBUGFS_H_

#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/mmc/card.h>
#include <linux/mmc/host.h>
#include <linux/debugfs.h>
#include <linux/uaccess.h>

#define ARRAY_SZ(arr) (sizeof(arr)/sizeof(arr[0]))

/* host entries */
typedef bool (mmc_host_debugfs_func)(struct mmc_host *host,
	struct dentry *root);

/* card entries */
typedef bool (mmc_card_debugfs_func)(struct mmc_card *card,
	struct dentry *root);

#define DEBUGFS_DECLARATIONS
#include "debugfs_entries.h"

#endif

