#include "debugfs.h"

#define TEMPLATE_DEFAULT_CONTENT_HOST "This is a template showing how to " \
	"add a debugfs file or an mmc host\n"

static char host_template[100] = TEMPLATE_DEFAULT_CONTENT_HOST;

static int mmc_jira_template_host_open(struct inode *inode, struct file *filp)
{
	struct mmc_host *host = inode->i_private;

	if (!host)
		return -EINVAL;

	filp->private_data = host;
	return 0;
}

static ssize_t mmc_jira_template_host_read(struct file *filp, char __user *ubuf,
	size_t cnt, loff_t *ppos)
{
	struct mmc_host *host = filp->private_data;
	int len, ret;

	if (!access_ok(VERIFY_READ, ubuf, cnt))
		return cnt;

	mmc_claim_host(host);
	len = strlen(host_template);
	ret = simple_read_from_buffer(ubuf, len, ppos, host_template, len);
	mmc_release_host(host);

	return ret;

}

static ssize_t mmc_jira_template_host_write(struct file *filp,
	const char __user *ubuf, size_t cnt, loff_t *ppos)
{
	struct mmc_host *host = filp->private_data;

	if (!access_ok(VERIFY_WRITE, ubuf, cnt))
		return cnt;

	if (cnt >= sizeof(host_template))
		return -EINVAL;

	mmc_claim_host(host);
	if (copy_from_user(host_template, ubuf, cnt))
		return -EFAULT;
	host_template[cnt] = '\0';
	mmc_release_host(host);

	return cnt;
}

static const struct file_operations mmc_jira_template_host = {
	.open = mmc_jira_template_host_open,
	.read = mmc_jira_template_host_read,
	.write = mmc_jira_template_host_write,
};

bool mmc_jira_template_debugfs_host(struct mmc_host *host, struct dentry *root)
{
	return debugfs_create_file("jira_template_host", S_IRUSR | S_IWUSR,
		root, host, &mmc_jira_template_host) ? true : false;
}

