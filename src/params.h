/*
  Copyright (C) 2012 Joseph J. Pfeiffer, Jr., Ph.D. <pfeiffer@cs.nmsu.edu>

  This program can be distributed under the terms of the GNU GPLv3.
  See the file COPYING.

  There are a couple of symbols that need to be #defined before
  #including all the headers.
*/

#ifndef _PARAMS_H_
#define _PARAMS_H_

// The FUSE API has been changed a number of times.  So, our code
// needs to define the version of the API that we assume.  As of this
// writing, the most current API version is 26
#define FUSE_USE_VERSION 26

// need this to get pwrite().  I have to use setvbuf() instead of
// setlinebuf() later in consequence.
#define _XOPEN_SOURCE 500

// maintain bbfs state in here
#include <limits.h>
#include <stdio.h>


#include "params.h"
#include "block.h"

#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <fuse.h>
#include <libgen.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>


#include "log.h"

struct inode {
        //struct list_head        i_list;              //list of i-nodes
        //struct list_head        i_data_entry;            //list of data entries [essentially the whole array]
        int   		            i_inode;              // i-node number
        uid_t                   i_uid;                // user_id of owner process - might need these for later
        gid_t                   i_gid;                // group_id of ownder process - might need these for later
        unsigned long           i_file_size_in_blocks;  //file size in blocks

        struct super_block      *i_sb;               //which superblock it points to
};

struct inode_bitmap{
	int bitmap[1024];
};
struct data_bitmap{
	int bitmap[1024];
};

struct inodes_table {
	struct inode table[1024]; //How big should the table be guys?
};

//This is the first block of the filysystem (considering a file system is just a really long array..which it actually is in memory)
struct super_block{
	int 						          total_i_nodes;
	int 						          total_data_entries; // total data entries
	unsigned long 				    sb_blocksize; 		// in bytes
	//struct super_operations 	s_op;  			/* superblock methods */
};

// Linux kernel super block operations from the original kernel
struct super_operations {
        struct inode *(*alloc_inode) (struct super_block *sb);
        void (*destroy_inode) (struct inode *);
        void (*read_inode) (struct inode *);
        void (*write_inode) (struct inode *, int);
        void (*delete_inode) (struct inode *);
        int (*sync_fs) (struct super_block *, int);
};

struct sfs_state {
    FILE *logfile;
    char *diskfile;
    pid_t pid;
    char * pid_path;
};
#define SFS_DATA ((struct sfs_state *) fuse_get_context()->private_data)

#endif
