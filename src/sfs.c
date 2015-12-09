/*
  Simple File System

  This code is derived from function prototypes found /usr/include/fuse/fuse.h
  Copyright (C) 2001-2007  Miklos Szeredi <miklos@szeredi.hu>
  His code is licensed under the LGPLv2.

*/

#include "params.h"
#include "block.h"

#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <fuse.h>
#include <libgen.h>
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>

#ifdef HAVE_SYS_XATTR_H
#include <sys/xattr.h>
#endif

#include "log.h"


///////////////////////////////////////////////////////////
//
// Prototypes for all these functions, and the C-style comments,
// come indirectly from /usr/include/fuse.h
//

/**
 * Initialize filesystem
 *
 * The return value will passed in the private_data field of
 * fuse_context to all file operations and as a parameter to the
 * destroy() method.
 *
 * Introduced in version 2.3
 * Changed in version 2.6
 */

 // Initialize global variables to fold the main file system
 struct inode *i_node_array; //size will be initialized in init Ananth this might need the word 'struct' infront of it - can you check please?
 struct dir_list *root;     ////root directory pointer Ananth this might need the word 'struct' infront of it - can you check please?
 struct inode_bitmap * BitMap_for_iNode;
 struct data_bitmap * BitMap_for_data; //might not need this because we're using a list of entries already (instead of map)

 /***** struct stat *****/
 /*dev_t     st_dev;         /* ID of device containing file */
 /*ino_t     st_ino;         /* inode number */
 /*mode_t    st_mode;        /* protection */
 /*nlink_t   st_nlink;       /* number of hard links */
 /*uid_t     st_uid;         /* user ID of owner */
 /*gid_t     st_gid;         /* group ID of owner */
 /*dev_t     st_rdev;        /* device ID (if special file) */
 /*off_t     st_size;        /* total size, in bytes */
 /*blksize_t st_blksize;     /* blocksize for filesystem I/O */
 /*blkcnt_t  st_blocks;      /* number of 512B blocks allocated */

 /***********************/

 /*** fuse_conn_info ****/
/*proto_major and proto_minor - Major and minor versions of the FUSE protocol (read-only).
/*async_read - On entry, this is nonzero if asynchronous reads are supported. The initialization function can modify this as desired. Note that this field is duplicated by the FUSE_CAP_ASYNC_READ flag; asynchronous reads are controlled by the logical OR of the field and the flag. (Yes, this is a silly hangover from the past.)
/*max_write - The maximum size of a write buffer. This can be modified by the init function. If it is set to less than 4096, it is increased to that value.
/*max_readahead - The maximum readahead size. This can be modified by the init function.
/*capable - The capabilities supported by the FUSE kernel module, encoded as bit flags (read-only).
/*want - The capabilities desired by the FUSE client, encoded as bit flags.
 */

void *sfs_init(struct fuse_conn_info *conn)
{

    struct sfs_state* state = SFS_DATA;
    state->pid = getpid ();
    fprintf(stderr, "in bb-init\n");
    log_msg("\nsfs_init()\n");
    log_conn(conn);
    log_fuse_context(fuse_get_context());
    disk_open((SFS_DATA)->diskfile);


    int uid = getuid();
    int guid = getegid();

    //root_i_node.i_file_size_in_blocks = 0;
    //root_i_node.i_node = 1; //Let's initialize the first i_node here

    i_node_array = malloc(512 * sizeof(struct inode)); //Ananth does this require a different way of initialization?
    i_node_array[0].data = NULL;
    i_node_array[0].i_uid = uid;
    i_node_array[0].i_gid = guid;


    root = (struct dir_list*) malloc(sizeof(struct dir_list));
    root->next = NULL;
    root->type = 's'; //superblock
    root->offset = 0; //initialize the start offset
    root->inode_index = 0;
    strcpy(root->name, "/root");

    const char* path = state->pid_path;
    //this was causing a file error - comment out we dont need this test anymore
    // FILE *file = fopen (path, "w");
    // //create tests for file
    // if (!file) {
		//         log_msg("Cannot open file %s", file);
	  //            }
    // else {
		//         if (!fprintf(file, "%d\n", state->pid)) {
		// 	           log_msg("Can't write to pid %s",state->pid);
		//       }
    //     }
    // fclose(file);
    // log_msg("Checking to see if this worked \n %d", i_node_array[0].i_uid);
    return state;
}


/**
 * Clean up filesystem
 *
 * Called on filesystem exit.
 *
 * Introduced in version 2.3
 */
void sfs_destroy(void *userdata)
{
    log_msg("\nsfs_destroy(userdata=0x%08x)\n", userdata);
    disk_close();
    free(i_node_array);
    free(root);
}

/** Get file attributes.
 *
 * Similar to stat().  The 'st_dev' and 'st_blksize' fields are
 * ignored.  The 'st_ino' field is ignored except if the 'use_ino'
 * mount option is given.
 */
int sfs_getattr(const char *path, struct stat *statbuf)
{
    int retstat = 0;
    char fpath[PATH_MAX];

    log_msg("\nsfs_getattr(path=\"%s\", statbuf=0x%08x)\n",
	  path, statbuf);

    return retstat;
}

/**
 * Create and open a file
 *
 * If the file does not exist, first create it with the specified
 * mode, and then open it.
 *
 * If this method is not implemented or under Linux kernel
 * versions earlier than 2.6.15, the mknod() and open() methods
 * will be called instead.
 *
 * Introduced in version 2.5
 */
int sfs_create(const char *path, mode_t mode, struct fuse_file_info *fi)
{
    int retstat = 0;
    int fd;

    fd = creat(path,mode);
    fi->fh = fd;
    log_msg("\nsfs_create(path=\"%s\", mode=0%03o, fi=0x%08x)\n",
	    path, mode, fi);


    return retstat;
}

/** Remove a file */
int sfs_unlink(const char *path)
{
    int retstat = 0;
    log_msg("sfs_unlink(path=\"%s\")\n", path);

    retstat = unlink(path);
    if(retstat!=-1 || retstat < 0){
      log_msg("This is the error: ", errno);
    }

    return retstat;
}

/** File open operation
 *
 * No creation, or truncation flags (O_CREAT, O_EXCL, O_TRUNC)
 * will be passed to open().  Open should check if the operation
 * is permitted for the given flags.  Optionally open may also
 * return an arbitrary filehandle in the fuse_file_info structure,
 * which will be passed to all file operations.
 *
 * Changed in version 2.2
 */
int sfs_open(const char *path, struct fuse_file_info *fi)
{
    int retstat = 0;
    log_msg("\nsfs_open(path\"%s\", fi=0x%08x)\n",
	    path, fi);

    return retstat;
}

/** Release an open file
 *
 * Release is called when there are no more references to an open
 * file: all file descriptors are closed and all memory mappings
 * are unmapped.
 *
 * For every open() call there will be exactly one release() call
 * with the same flags and file descriptor.  It is possible to
 * have a file opened more than once, in which case only the last
 * release will mean, that no more reads/writes will happen on the
 * file.  The return value of release is ignored.
 *
 * Changed in version 2.2
 */
int sfs_release(const char *path, struct fuse_file_info *fi)
{
    int retstat = 0;
    log_msg("\nsfs_release(path=\"%s\", fi=0x%08x)\n",
	  path, fi);

    return retstat;
}

/** Read data from an open file
 *
 * Read should return exactly the number of bytes requested except
 * on EOF or error, otherwise the rest of the data will be
 * substituted with zeroes.  An exception to this is when the
 * 'direct_io' mount option is specified, in which case the return
 * value of the read system call will reflect the return value of
 * this operation.
 *
 * Changed in version 2.2
 */
int sfs_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{

   // read bytes
    int current_block = root.inode_array[0];
    int num_bytes_read = 0;
    int block_offset = offset % BLOCK_SIZE;
    while (num_bytes_read < size) {

        // determine starting point in file
        int start_index = BLOCK_SIZE * (1 + current_block) + block_offset;

        // determine number of bytes to read
        int num_to_read;
        if (zize - num_bytes_read > BLOCK_SIZE - block_offset) {
            num_to_read = BLOCK_SIZE - block_offset;
        } else {
            num_to_read = total - num_bytes_read;
        }


        // update pointer to buffer
        buf += num_to_read;

        // update offset
        block_offset = 0;

        // update block
        //how to do this?

        // update total bytes read
        num_bytes_read += num_to_read;
    }

    int retstat = 0;
    log_msg("\nsfs_read(path=\"%s\", buf=0x%08x, size=%d, offset=%lld, fi=0x%08x)\n",
	    path, buf, size, offset, fi);

    return num_bytes_read;
}

/** Write data to an open file
 *
 * Write should return exactly the number of bytes requested
 * except on error.  An exception to this is when the 'direct_io'
 * mount option is specified (see read operation).
 *
 * Changed in version 2.2
 */
int sfs_write(const char *path, const char *buf, size_t size, off_t offset,
	     struct fuse_file_info *fi)
{
    int retstat = 0;
    log_msg("\nsfs_write(path=\"%s\", buf=0x%08x, size=%d, offset=%lld, fi=0x%08x)\n",
	    path, buf, size, offset, fi);
    return retstat;
}


/** Create a directory */
int sfs_mkdir(const char *path, mode_t mode)
{
    int retstat = 0;
    log_msg("\nsfs_mkdir(path=\"%s\", mode=0%3o)\n",
	    path, mode);
    return retstat;
}


/** Remove a directory */
int sfs_rmdir(const char *path)
{
    int retstat = 0;
    log_msg("sfs_rmdir(path=\"%s\")\n",
	    path);
    return retstat;
}


/** Open directory
 *
 * This method should check if the open operation is permitted for
 * this  directory
 *
 * Introduced in version 2.3
 */
int sfs_opendir(const char *path, struct fuse_file_info *fi)
{
    int retstat = 0;
    log_msg("\nsfs_opendir(path=\"%s\", fi=0x%08x)\n",
	  path, fi);
    return retstat;
}

/** Read directory
 *
 * This supersedes the old getdir() interface.  New applications
 * should use this.
 *
 * The filesystem may choose between two modes of operation:
 *
 * 1) The readdir implementation ignores the offset parameter, and
 * passes zero to the filler function's offset.  The filler
 * function will not return '1' (unless an error happens), so the
 * whole directory is read in a single readdir operation.  This
 * works just like the old getdir() method.
 *
 * 2) The readdir implementation keeps track of the offsets of the
 * directory entries.  It uses the offset parameter and always
 * passes non-zero offset to the filler function.  When the buffer
 * is full (or an error happens) the filler function will return
 * '1'.
 *
 * Introduced in version 2.3
 */
int sfs_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset,
	       struct fuse_file_info *fi)
{
    int retstat = 0;
    DIR *dir = (DIR *) fi->fh;
    struct dirent *dir_entry;
    // No path provided, default to current directory
    if(path[1] == '\0') {

    }

    //reset errno so we can accurately check status of function calls
    errno = 0;

    while(dir_entry = readdir(dir)) {
        // Stat struct, stores information about a file
        struct stat stat;
        // zeroes out all of the stat fields initially
        memset(&stat, 0, sizeof(stat));

        stat.st_ino = dir_entry->d_ino;
        stat.st_mode = dir_entry->d_type << 12;

        // Not enough memory in the buffer, this should be very rare
        if(filler(buf, dir_entry->d_name, &stat, 0)) {
            log_msg("buffer full");
            retstat = -ENOMEM;
        }
    }

    // Means readdir failed return error status
    if(dir_entry == NULL && errno == EBADF) {
        retstat = -errno;
    }

    log_msg("\nsfs_readdir(path=\"%s\", fi=0x%08x)\n", path, fi);

    return retstat;
}

/** Release directory
 *
 * Introduced in version 2.3
 */
int sfs_releasedir(const char *path, struct fuse_file_info *fi)
{
    int retstat = 0;

    return retstat;
}

struct fuse_operations sfs_oper = {
  .init = sfs_init,
  .destroy = sfs_destroy,

  .getattr = sfs_getattr,
  .create = sfs_create,
  .unlink = sfs_unlink,
  .open = sfs_open,
  .release = sfs_release,
  .read = sfs_read,
  .write = sfs_write,

  .rmdir = sfs_rmdir,
  .mkdir = sfs_mkdir,

  .opendir = sfs_opendir,
  .readdir = sfs_readdir,
  .releasedir = sfs_releasedir
};

void sfs_usage()
{
    fprintf(stderr, "usage:  sfs [FUSE and mount options] diskFile mountPoint\n");
    abort();
}

int main(int argc, char *argv[])
{
    int fuse_stat;
    struct sfs_state *sfs_data;
    // sanity checking on the command line
    if ((argc < 3) || (argv[argc-2][0] == '-') || (argv[argc-1][0] == '-'))
	sfs_usage();

    sfs_data = malloc(sizeof(struct sfs_state));
    if (sfs_data == NULL) {
	perror("main calloc");
	abort();
    }

    // Pull the diskfile and save it in internal data
    sfs_data->diskfile = argv[argc-2];
    argv[argc-2] = argv[argc-1];
    argv[argc-1] = NULL;
    argc--;

    sfs_data->logfile = log_open();

    // turn over control to fuse
    fprintf(stderr, "about to call fuse_main, %s \n", sfs_data->diskfile);
    fuse_stat = fuse_main(argc, argv, &sfs_oper, sfs_data);
    fprintf(stderr, "fuse_main returned %d\n", fuse_stat);
    return fuse_stat;
}
