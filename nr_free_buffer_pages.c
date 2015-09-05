#include <linux/swap.h>

#include "debugfs.h"

static ssize_t mmc_nr_free_buffer_pages_read(struct file *filp,
	char __user *ubuf, size_t cnt, loff_t *ppos)
{
	char buf[50];
	int len;

	if (!access_ok(VERIFY_READ, ubuf, cnt))
		return cnt;

	snprintf(buf, sizeof(buf), "%lu\n", nr_free_buffer_pages());
	len = strnlen(buf, cnt);
	return simple_read_from_buffer(ubuf, len, ppos, buf, len);
}

static const struct file_operations mmc_dbg_nr_free_buffer_pages_fops = {
	.read		= mmc_nr_free_buffer_pages_read,
};

bool mmc_nr_free_buffer_pages_debugfs(struct mmc_host *host,
	struct dentry *root)
{
	if (!host)
		return true;

	return debugfs_create_file("nr_free_buffer_pages", S_IRUSR, root, host,
		&mmc_dbg_nr_free_buffer_pages_fops) ? true : false;
}

