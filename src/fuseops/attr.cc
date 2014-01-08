#include "main.h"
#include "debug.h"
#include <sys/types.h>

static void fillattr(struct stat *attr, MetadataInfo * mdi)
{
	attr->st_atime = mdi->pbuf()->atime();
	attr->st_mtime = mdi->pbuf()->mtime();
	attr->st_ctime = mdi->pbuf()->ctime();
	attr->st_uid   = mdi->pbuf()->id_user();
	attr->st_gid   = mdi->pbuf()->id_group();
	attr->st_mode  = mdi->pbuf()->mode();
	attr->st_nlink = mdi->pbuf()->link_count();
	attr->st_size  = mdi->pbuf()->size();
	attr->st_blocks= mdi->pbuf()->blocks();
}

/**
 * Get attributes from an open file
 *
 * This method is called instead of the getattr() method if the
 * file information is available.
 *
 * Currently this is only called after the create() method if that
 * is implemented (see above).  Later it may be called for
 * invocations of fstat() too.
 *
 * Introduced in version 2.5
 */
int pok_fgetattr(const char *user_path, struct stat *attr, struct fuse_file_info *fi)
{
	pok_debug("Getting attribtues of user path %s",user_path);
	MetadataInfo * mdi = reinterpret_cast<MetadataInfo *>(fi->fh);
	if(!mdi){
		pok_error("Read request for user path '%s' without metadata_info structure", user_path);
		return -EINVAL;
	}
	fillattr(attr,mdi);
	return 0;
}

/** Get file attributes.
 *
 * Similar to stat().  The 'st_dev' and 'st_blksize' fields are
 * ignored.	 The 'st_ino' field is ignored except if the 'use_ino'
 * mount option is given.
 */
int pok_getattr(const char *user_path, struct stat *attr)
{
	pok_debug("Getting attribtues of user path %s",user_path);
	std::unique_ptr<MetadataInfo> mdi(new MetadataInfo());
	int err = lookup(user_path, mdi);
	if (err){
		pok_debug("lookup returned error code %d",err);
		return err;
	}
	fillattr(attr,mdi.get());
	return 0;
}

/**
 * Change the access and modification times of a file with
 * nanosecond resolution
 *
 * Introduced in version 2.6
 */
int pok_utimens	(const char *user_path, const struct timespec tv[2])
{
	std::unique_ptr<MetadataInfo> mdi(new MetadataInfo());
	int err = lookup(user_path, mdi);
	if (err){
		pok_debug("lookup returned error code %d",err);
		return err;
	}
	mdi->pbuf()->set_atime(	tv[0].tv_sec );
	mdi->pbuf()->set_mtime( tv[1].tv_sec );
	NamespaceStatus status = PRIV->nspace->putMD(mdi.get());
	if(status.notOk())
		return -EIO;
	return 0;
}

/** Get file system statistics
 *
 * The 'f_frsize', 'f_favail', 'f_fsid' and 'f_flag' fields are ignored
 *
 * Replaced 'struct statfs' parameter with 'struct statvfs' in
 * version 2.5
 */
int pok_statfs (const char *user_path, struct statvfs *s)
{
	/* Do we want to keep an inodecount and a blockcount?
	 * If yes we'd probably update infrequently (e.g. on client unmount)  */
	s->f_bsize  = 1024*1024;
	return 0;
}