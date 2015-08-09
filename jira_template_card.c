#include "debugfs.h"

#define TEMPLATE_DEFAULT_CONTENT_CARD "This is a template showing how to " \
	"add a debugfs file or an mmc card\n"

static char card_template[100] = TEMPLATE_DEFAULT_CONTENT_CARD;

static int mmc_jira_template_card_open(struct inode *inode, struct file *filp)
{
	struct mmc_card *card = inode->i_private;

	if (!card)
		return -EINVAL;

	filp->private_data = card;
	return 0;
}

static ssize_t mmc_jira_template_card_read(struct file *filp, char __user *ubuf,
	size_t cnt, loff_t *ppos)
{
	struct mmc_card *card = filp->private_data;
	struct mmc_host *host = card->host;
	int len, ret;

	if (!access_ok(VERIFY_READ, ubuf, cnt))
		return cnt;

	mmc_claim_host(host);
	len = strlen(card_template);
	ret = simple_read_from_buffer(ubuf, len, ppos, card_template, len);
	mmc_release_host(host);

	return ret;
}

static ssize_t mmc_jira_template_card_write(struct file *filp,
	const char __user *ubuf, size_t cnt, loff_t *ppos)
{
	struct mmc_card *card = filp->private_data;
	struct mmc_host *host = card->host;

	if (!access_ok(VERIFY_WRITE, ubuf, cnt))
		return cnt;

	if (cnt >= sizeof(card_template))
		return -EINVAL;

	mmc_claim_host(host);
	if (copy_from_user(card_template, ubuf, cnt))
		return -EFAULT;
	card_template[cnt] = '\0';
	mmc_release_host(host);

	return cnt;
}

static const struct file_operations mmc_jira_template_card = {
	.open = mmc_jira_template_card_open,
	.read = mmc_jira_template_card_read,
	.write = mmc_jira_template_card_write,
};

bool mmc_jira_template_debugfs_card(struct mmc_card *card, struct dentry *root)
{
	if (!mmc_card_mmc(card))
		return true;

	return debugfs_create_file("jira_template_card", S_IRUSR | S_IWUSR,
		root, card, &mmc_jira_template_card) ? true : false;
}

