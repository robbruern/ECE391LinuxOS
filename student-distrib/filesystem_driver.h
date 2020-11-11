/* filesystem_driver.h - File system Driver.
 * vim:ts=4 noexpandtab
 */
#pragma once

#include "types.h"
#include "process_control.h"
#include "lib.h"
#include "terminal.h"


#define MAX_DIR_ENTRIES     63      // Maximum number of directory entries.
#define FILE_SIZE           4096    // File Size
#define FILENAME_SIZE       32      // Filename size.
#define IN_USE              1

// Directory Entry Structure.
typedef struct dir_entry
{
    char filename[FILENAME_SIZE];
    uint32_t file_type;
    uint32_t inode_num;
    char reserved[24];
} dir_entry_t;

// Boot Block Structure.
typedef struct boot_block
{
    uint32_t num_dir_entries;
    uint32_t inodes;
    uint32_t data_blocks;
    char reserved[52];
    dir_entry_t dir_entries[MAX_DIR_ENTRIES];
} boot_block_t;

// Node Block Structure.
typedef struct node_block
{
    uint32_t B_length;
    uint32_t data_blocks[1023];
} node_block_t;

// Data Block Structure.
typedef struct data_block
{
    uint32_t data[1024];
} data_block_t;

// Umbrella Block Structure - used to tag nodes to data blocks.
typedef struct umbrella_block
{
    node_block_t *node_list;
    data_block_t *data_list;
} umbrella_block_t;

node_block_t * node_list;
data_block_t * data_list;

extern void boot_block_init(unsigned int addr);

extern int32_t read_dentry_by_name(const uint8_t *fname, dir_entry_t *dentry);
extern int32_t read_dentry_by_index(unsigned int index, dir_entry_t *dentry);

extern int32_t read_data (uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length);

extern int32_t file_open(const uint8_t * filename);
extern int32_t file_close(int32_t fd);
extern int32_t file_write(int32_t fd, const void* buf, int32_t nbytes);
extern int32_t file_read(int32_t fd, void* buf, int32_t nbytes);

extern int32_t directory_open(const uint8_t * filename);
extern int32_t directory_close(int32_t fd);
extern int32_t directory_write(int32_t fd, const void* buf, int32_t nbytes);
extern int32_t directory_read(int32_t fd, void* buf, int32_t nbytes);
