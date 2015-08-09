#include <linux/device.h>
#include <linux/debugfs.h>
#include <linux/mmc/mh_debugfs.h>

#include <linux/kut_device.h>
#include <linux/mmc/kut_bus.h>
#include <linux/mmc/kut_host.h>
#include <linux/kut_namei.h>

#include <unit_test.h>

struct device platform;
struct device ram_console;
static struct kunit_host *hc;
static struct mmc_host *host;
static struct mmc_card *card;

static struct dentry *mmc_debugfs_dentry_lookup(bool is_host, char *file)
{
	char path[PATH_MAX] = {0};

	snprintf(path, sizeof(path), "%s/%s",
		is_host ? "host/mh" : "host/card/mh", file);

	return kut_dentry_lookup(NULL, path);
}

static int mmc_debugfs_read_test(bool is_host, char *file, char *expected)
{
	struct dentry *dentry;
	char buf[PAGE_SIZE] = {0};

	dentry = mmc_debugfs_dentry_lookup(is_host, file);
	if (!dentry)
		return -1;

	kut_dentry_read(dentry, buf, sizeof(buf));
	printf("%s", buf);

	if (strcmp(buf, expected))
		return -1;

	return 0;
}

static int mmc_debugfs_write_test(bool is_host, char *file, char *data)
{
	struct dentry *dentry;
	char buf[PAGE_SIZE] = {0};

	dentry = mmc_debugfs_dentry_lookup(is_host, file);
	if (!dentry)
		return -1;

	snprintf(buf, sizeof(buf), data, strlen(data));
	kut_dentry_write(dentry, buf, strlen(data) + 1);
	return mmc_debugfs_read_test(is_host, file, data);
}

static int mmc_debugfs_template_host(void)
{
	int ret;

	/* read test */
	ret = mmc_debugfs_read_test(true, "jira_template_host",
		"This is a template showing how to add a debugfs file or an " \
		"mmc host\n");
	if (ret)
		return -1;

	/* write test */
	ret = mmc_debugfs_write_test(true, "jira_template_host",
		"Writing something much shorter for the host\n");
	if (ret)
		return -1;

	return 0;
}

static int mmc_debugfs_template_card(void)
{
	int ret;

	/* read test */
	ret = mmc_debugfs_read_test(false, "jira_template_card",
		"This is a template showing how to add a debugfs file or an " \
		"mmc card\n");
	if (ret)
		return -1;

	/* write test */
	ret = mmc_debugfs_write_test(false, "jira_template_card",
		"Writing something much shorter for the card\n");
	if (ret)
		return -1;

	return 0;
}

static int mmc_debugfs_dev_tree(void)
{
	return mmc_debugfs_read_test(false, "dev_tree",
		".\n" \
		"|\n" \
		"+-- platform\n" \
		"    |\n" \
		"    +-- kunit-hc.0\n" \
		"    |   |\n" \
		"    |   +-- mmc0 [host]\n" \
		"    |       |\n" \
		"    |       +-- mmc0:0001 [card]\n" \
		"    |           |\n" \
		"    |           +-- mmcblk0\n" \
		"    |               |\n" \
		"    |               +-- mmcblk0p1\n" \
		"    |               |\n" \
		"    |               +-- mmcblk0p2\n" \
		"    |               |\n" \
		"    |               +-- mmcblk0p3\n" \
		"    |               |\n" \
		"    |               +-- mmcblk0p4\n" \
		"    |               |\n" \
		"    |               +-- mmcblk0p5\n" \
		"    |               |\n" \
		"    |               +-- mmcblk0p6\n" \
		"    |               |\n" \
		"    |               +-- mmcblk0p7\n" \
		"    |               |\n" \
		"    |               +-- mmcblk0p8\n" \
		"    |\n" \
		"    +-- ram_console\n");
}
 
static struct single_test mmc_debugfs_tests[] = {
	{
		description: "tempate host: verify host debugfs file " \
			"operations",
		func: mmc_debugfs_template_host,
	},
	{
		description: "tempate card: verify card debugfs file " \
			"operations",
		func: mmc_debugfs_template_card,
	},
	{
		description: "dev_tree: verify correct device tree hierarchy",
		func: mmc_debugfs_dev_tree,
	},
};

static int mmc_debugfs_uninit(void)
{
	debugfs_remove_recursive(&kern_root);
	kut_mmc_uninit(hc, host, card);
	kut_dev_uninit(&platform);

	return 0;
}

static int mmc_debugfs_init(void)
{
	struct dentry *root;

	/* Create the following debugsf directory tree:
	 *
	 * .
	 * └── host
	 *     ├── card
	 *     │   └── mh
	 *     │       ├── dev_tree
	 *     │       ├── jira_template_card
	 *     │       └── ...
	 *     └── mh
	 *         ├── jira_template_host
	 *         └── ...
	 */

	if (!kut_dev_init(&platform, NULL, "platform"))
		goto error;

	/* allocate host controller, host, card and 8 partitions */
	if (kut_mmc_init(&platform, &hc, &host, &card, 8))
		goto error;

	/* some other device to complicate the tree structure */
	if (!kut_dev_init(&ram_console, &platform, "ram_console"))
		goto error;

	/* create host debugfs directory */
	if (!(root = debugfs_create_dir("host", NULL)))
		goto error;

	/* create host mh debugfs directory */
	if (!mmc_mh_debugfs_host(host, root))
		goto error;

	/* create card debugfs directory */
	if (!(root = debugfs_create_dir("card", root)))
		goto error;

	/* create card mh debugfs directory */
	if (!mmc_mh_debugfs_card(card, root))
		goto error;

	return 0;

error:
	mmc_debugfs_uninit();
	return -1;
}

struct unit_test ut_mmc_debugfs = {
	.module = "mmc_debugfs",
	.description = "Various debug FS files",
	.pre_all_tests = mmc_debugfs_init,
	.post_all_tests = mmc_debugfs_uninit,
	.tests = mmc_debugfs_tests,
	.count = ARRAY_SZ(mmc_debugfs_tests),
};

