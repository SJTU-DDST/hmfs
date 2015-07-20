#ifndef _LINUX_HMFS_H
#define _LINUX_HMFS_H

#include <linux/slab.h>
#include <linux/types.h>
#include "hmfs_fs.h"

#ifdef CONFIG_HMFS_CHECK_FS
#define hmfs_bug_on(sbi, condition)	BUG_ON(condition)
#define hmfs_down_write(x, y)	down_write_nest_lock(x, y)
#else
#define hmfs_bug_on(sbi, condition)					\
	do {								\
		if (unlikely(condition)) {				\
			WARN_ON(1);					\
			set_sbi_flag(sbi, SBI_NEED_FSCK);		\
		}							\
	} while (0)
#define hmfs_down_write(x, y)	down_write(x)
#endif

#define MAX_DIR_RA_PAGES	4	/* maximum ra pages of dir */

/*
 * For INODE and NODE manager
 */
/* for directory operations */
struct hmfs_dentry_ptr {
	const void *bitmap;
	struct hmfs_dir_entry *dentry;
	__u8 (*filename)[HMFS_SLOT_LEN];
	int max;
};

static inline void make_dentry_ptr(struct hmfs_dentry_ptr *d,
					void *src, int type)
{
	if (type == 1) {
		struct hmfs_dentry_block *t = (struct hmfs_dentry_block *)src;
		d->max = NR_DENTRY_IN_BLOCK;
		d->bitmap = &t->dentry_bitmap;
		d->dentry = t->dentry;
		d->filename = t->filename;
	} else {
		struct hmfs_inline_dentry *t = (struct hmfs_inline_dentry *)src;
		d->max = NR_INLINE_DENTRY;
		d->bitmap = &t->dentry_bitmap;
		d->dentry = t->dentry;
		d->filename = t->filename;
	}
}



typedef unsigned long nid_t;

struct checkpoint_info {
	unsigned int version;

	unsigned long cur_node_segno;
	unsigned int cur_node_blkoff;

	unsigned long cur_data_segno;
	unsigned int cur_data_blkoff;

	unsigned valid_inode_count;

	unsigned last_checkpoint_addr;

	struct hmfs_checkpoint *cp;
	struct page *cp_page;
};

struct hmfs_sb_info {
	struct super_block *sb;

	/* 1. location info  */
	phys_addr_t phys_addr;	//get from user mount                   [hmfs_parse_options]
	void *virt_addr;	//hmfs_superblock & also HMFS address   [ioremap]

	unsigned long initsize;
	unsigned long s_mount_opt;

	unsigned long page_count;
	unsigned long segment_count;

	struct checkpoint_info *cp;

	unsigned long ssa_addr;
	unsigned long main_addr_start;
	unsigned long main_addr_end;

	/**
	 * statiatic infomation, for debugfs
	 */
	struct hmfs_stat_info *stat_info;

	struct inode *nat_inode;
	struct inode *sit_inode;
	struct inode *ssa_inode;
};

struct hmfs_inode_info {
	struct inode vfs_inode;	/* vfs inode */
	unsigned char i_dir_level;/* use for dentry level for large dir */
	hmfs_hash_t chash;		/* hash value of given file name */
	unsigned int i_current_depth;	/* use only in directory structure */
	unsigned int clevel;		/* maximum level of given file name */
	/* Use below internally in hmfs*/
	unsigned long flags;		/* use to pass per-file flags */
	struct rw_semaphore i_sem;	/* protect fi info */
	unsigned int i_pino;		/* parent inode number */

};

struct hmfs_stat_info {
	struct list_head stat_list;
	struct hmfs_sb_info *sbi;
};

struct node_info {
	nid_t nid;
	nid_t ino;
	unsigned long blk_addr;
	unsigned int version;
};

/* used for hmfs_inode_info->flags */
enum {
	FI_NEW_INODE,		/* indicate newly allocated inode */
	FI_DIRTY_INODE,		/* indicate inode is dirty or not */
	FI_DIRTY_DIR,		/* indicate directory has dirty pages */
	FI_INC_LINK,		/* need to increment i_nlink */
	FI_ACL_MODE,		/* indicate acl mode */
	FI_NO_ALLOC,		/* should not allocate any blocks */
	FI_UPDATE_DIR,		/* should update inode block for consistency */
	FI_DELAY_IPUT,		/* used for the recovery */
	FI_NO_EXTENT,		/* not to use the extent cache */
	FI_INLINE_XATTR,	/* used for inline xattr */
	FI_INLINE_DATA,		/* used for inline data*/
	FI_INLINE_DENTRY,	/* used for inline dentry */
	FI_APPEND_WRITE,	/* inode has appended data */
	FI_UPDATE_WRITE,	/* inode has in-place-update data */
	FI_NEED_IPU,		/* used for ipu per file */
	FI_ATOMIC_FILE,		/* indicate atomic file */
	FI_VOLATILE_FILE,	/* indicate volatile file */
	FI_FIRST_BLOCK_WRITTEN,	/* indicate #0 data block was written */
	FI_DROP_CACHE,		/* drop dirty page cache */
	FI_DATA_EXIST,		/* indicate data exists */
	FI_INLINE_DOTS,		/* indicate inline dot dentries */
};

enum page_type {
	DATA,
	NODE,
	META,
	NR_PAGE_TYPE,
	META_FLUSH,
	INMEM,		/* the below types are used by tracepoints only. */
	INMEM_DROP,
	IPU,
	OPU,
};

extern const struct file_operations hmfs_file_operations;
extern const struct file_operations hmfs_dir_operations;

extern const struct inode_operations hmfs_file_inode_operations;
extern const struct inode_operations hmfs_dir_inode_operations;
extern const struct inode_operations hmfs_symlink_inode_operations;
extern const struct inode_operations hmfs_special_inode_operations;

extern const struct address_space_operations hmfs_dblock_aops;
extern const struct address_space_operations hmfs_nat_aops;
extern const struct address_space_operations hmfs_sit_aops;
extern const struct address_space_operations hmfs_ssa_aops;
/*
 * Inline functions
 */
static inline struct hmfs_inode_info *HMFS_I(struct inode *inode)
{
	return container_of(inode, struct hmfs_inode_info, vfs_inode);
}

static inline struct hmfs_sb_info *HMFS_SB(struct super_block *sb)
{
	return sb->s_fs_info;
}

static inline struct hmfs_inode *HMFS_INODE(struct page *page)
{
	return &((struct hmfs_node *)page_address(page))->i;
}

static inline struct checkpoint_info *CURCP_I(struct hmfs_sb_info *sbi)
{
		//TODO  
		return sbi->cp;
}

static inline void *ADDR(struct hmfs_sb_info *sbi, unsigned logic_addr)
{
	return (sbi->virt_addr + logic_addr);
}

static inline nid_t START_NID(nid_t nid)
{
	//TODO
	return nid;
}

static inline struct hmfs_sb_info *HMFS_I_SB(struct inode *inode)
{
	return HMFS_SB(inode->i_sb);
}

static inline struct hmfs_sb_info *HMFS_M_SB(struct address_space *mapping)
{
	return HMFS_I_SB(mapping->host);
}

static inline struct hmfs_sb_info *HMFS_P_SB(struct page *page)
{
	return HMFS_M_SB(page->mapping);
}

static inline void set_inode_flag(struct hmfs_inode_info *fi, int flag)
{
	if (!test_bit(flag, &fi->flags))
		set_bit(flag, &fi->flags);
}

static inline void clear_inode_flag(struct hmfs_inode_info *fi, int flag)
{
	if (test_bit(flag, &fi->flags))
		clear_bit(flag, &fi->flags);
}

/* define prototype function */

/* inode.c */
struct inode *hmfs_iget(struct super_block *sb, unsigned long ino);

/* debug.c */
void hmfs_create_root_stat(void);
void hmfs_destroy_root_stat(void);
int hmfs_build_stats(struct hmfs_sb_info *sbi);
void hmfs_destroy_stats(struct hmfs_sb_info *sbi);

/* node.c */
int build_node_manager(struct hmfs_sb_info *sbi);
void destroy_node_manager(struct hmfs_sb_info *sbi);
void get_node_info(struct hmfs_sb_info *sbi, nid_t nid, struct node_info *ni);
void node_info_from_raw_nat(struct node_info *ni, struct hmfs_nat_entry *ne);

/* checkpoint.c */
void init_checkpoint_manager(struct hmfs_sb_info *sbi);
void destroy_checkpoint_manager(struct hmfs_sb_info *sbi);
int lookup_journal_in_cp(struct checkpoint_info *cp_info, unsigned int type,
			 nid_t nid, int alloc);
struct hmfs_nat_entry nat_in_journal(struct checkpoint_info *cp_info,
				     int index);

static inline void hmfs_put_page(struct page *page, int unlock)
{
	if (!page)
		return;

	if (unlock) {
		//hmfs_bug_on(HMFS_P_SB(page), !PageLocked(page));
		unlock_page(page);
	}
	page_cache_release(page);
}

static inline int is_inode_flag_set(struct hmfs_inode_info *fi, int flag)
{
	return test_bit(flag, &fi->flags);
}

static inline int hmfs_has_inline_dentry(struct inode *inode)
{
	return is_inode_flag_set(HMFS_I(inode), FI_INLINE_DENTRY);
}

static inline void hmfs_dentry_kunmap(struct inode *dir, struct page *page)
{
	if (!hmfs_has_inline_dentry(dir))
		kunmap(page);
}
#endif
