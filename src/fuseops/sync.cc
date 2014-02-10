#include "main.h"
#include "kinetic_helper.h"
/* No Functionality in here yet... shouldn't be needed unless we start some caching / buffering somewhere */

/** Synchronize file contents
 *
 * If the datasync parameter is non-zero, then only the user data
 * should be flushed, not the meta data.
 *
 * Changed in version 2.2
 */
int pok_fsync(const char *user_path, int datasync, struct fuse_file_info *fi)
{
    std::shared_ptr<MetadataInfo> mdi;
    int err = lookup(user_path, mdi);
    if( err) return err;

    /* The only reason for dirty data or metadata is an aggregated write operation.  */
    if(mdi->isDirty()){
        if (!datasync){
             err = put_metadata(mdi);
             if(err == -EAGAIN) return pok_fsync(user_path, datasync, fi);
        }
        if(!err) err = put_data(mdi->getAggregate());
    }
    return err;
}

/** Synchronize directory contents
 *
 * If the datasync parameter is non-zero, then only the user data
 * should be flushed, not the meta data
 *
 * Introduced in version 2.3
 */
int pok_fsyncdir(const char *user_path, int datasync, struct fuse_file_info *fi)
{
    std::shared_ptr<MetadataInfo> mdi;
    int err = lookup(user_path, mdi);
    if( err) return err;

    put_metadata(mdi);
    return 0;
}

/** Possibly flush cached data
 *
 * BIG NOTE: This is not equivalent to fsync().  It's not a
 * request to sync dirty data.
 *
 * Flush is called on each close() of a file descriptor.  So if a
 * filesystem wants to return write errors in close() and the file
 * has cached dirty data, this is a good place to write back data
 * and return any errors.  Since many applications ignore close()
 * errors this is not always useful.
 *
 * NOTE: The flush() method may be called more than once for each
 * open().	This happens if more than one file descriptor refers
 * to an opened file due to dup(), dup2() or fork() calls.	It is
 * not possible to determine if a flush is final, so each flush
 * should be treated equally.  Multiple write-flush sequences are
 * relatively rare, so this shouldn't be a problem.
 *
 * Filesystems shouldn't assume that flush will always be called
 * after some writes, or that if will be called at all.
 *
 * Changed in version 2.2
 */
int pok_flush(const char *user_path, struct fuse_file_info *fi)
{
    /* This seems like too much of a random place to do much of anything. */
    return 0;
}
