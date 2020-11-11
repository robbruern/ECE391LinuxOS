/* filesystem_driver.h - File system Driver.
 * vim:ts=4 noexpandtab
 */
#include "filesystem_driver.h"

static boot_block_t *casted_block; //boot block of filesystem

/*
 *   boot_block_init
 *   DESCRIPTION: Initializes structs to represent the filesystem at the passed
 *   in address
 *   INPUTS: unsigned in addr: address of the filesystem
 *   OUTPUTS: none
 *   RETURN VALUE: 0 on success, -1 on failure
 *   SIDE EFFECTS: none
 */
void boot_block_init(unsigned int addr)
{
    /** Point the statically defined structs to the address passed in */
    casted_block = (boot_block_t *)addr;
    node_list = (node_block_t *)(addr + FILE_SIZE);
    data_list = (data_block_t *)(addr + FILE_SIZE + (casted_block->inodes * FILE_SIZE));
}

/*
 *   read_dentry_by_name
 *   DESCRIPTION: Populates the passed in dentry struct
 *   INPUTS: unsigned uint8_t* fname: name of the dentry
 *   dir_entry_t* dentry: empty dentry struct
 *   OUTPUTS: none
 *   RETURN VALUE: 0 on success, -1 on failure
 *   SIDE EFFECTS: none
 */
int32_t read_dentry_by_name(const uint8_t *fname, dir_entry_t *dentry)
{
    if (strlen((const int8_t*)fname) == 0) {
        return FAILURE;
    }
    if (strlen((const int8_t*)fname) > 32) {
        return FAILURE;
    }

    int i = 0;
    /** Iterate over all the d_entries */
    for (i = 0; i < MAX_DIR_ENTRIES; i++)
    {
        /** Check to see if the filename exists. */
        int8_t *current_filename = (int8_t *)casted_block->dir_entries[i].filename;
        if (current_filename[0] == '\0') {
            continue;
        }

        /** If the names are the same, set the dentry and return 0 */
        if (0 == strncmp(current_filename, (char *)fname, FILENAME_SIZE))
        {
            *dentry = casted_block->dir_entries[i];
            return SUCCESS;
        }

    }
    /** If none are found, return -1 */
    return FAILURE;
}

/*
 *   read_dentry_by_index
 *   DESCRIPTION: Populates the passed in dentry struct
 *   INPUTS: unsigned int index: index of the dentry
 *   dir_entry_t* dentry: empty dentry struct
 *   OUTPUTS: none
 *   RETURN VALUE: 0 on success, -1 on failure
 *   SIDE EFFECTS: none
 */
int32_t read_dentry_by_index(unsigned int index, dir_entry_t *dentry)
{
    /** Get the total number of inodes */
    uint32_t max_inodes = casted_block->inodes;

    /** Check for valid index */
    if (index > (max_inodes - 1)) {
        return -1;
    }
    if (index < 0){
        return -1;
    }

    /** Populate dentry and return */
    *dentry = casted_block->dir_entries[index];
    return 0;
}

/*
 * read_data
 *   DESCRIPTION: reads data in a file by looking at the providede inode and offset
 *   and length of desired data wanted and writes it into a buffer passed into the function
 *   INPUTS: uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length
 *   OUTPUTS: copies file data into the buffer
 *   RETURN VALUE: 0 on success, -1 on failure
 *   SIDE EFFECTS: none
 */
int32_t read_data (uint32_t fd, uint32_t offset, uint8_t* buf, uint32_t length)
{
    uint32_t max_inodes;    /** max amount of inodes */
    unsigned int curr;      /** variable to be used to access the index of data_pointer */
    uint32_t num;           /** length of data in inode */
    uint8_t* data_pointer;  /** pointer used for accessing file data */
    int i;                  /** loop iteration variable */
    int j;                  /** loop iteration variable */
    int loop;               /** var to switch data blocks */
    uint32_t inode;         /** inode num from fd */
    inode = control_blocks[current_pid].fd_table[fd].inode;
    /** if inode out of range, return -1 */
    max_inodes = casted_block->inodes;
    if (inode > max_inodes) {
        return -1;
    }
    curr = node_list[inode].data_blocks[offset / FILE_SIZE];
    num = node_list[inode].B_length;
    if (num == 0) {

        return 0;
    }

	if(control_blocks[current_pid].fd_table[fd].file_position > num){
		return 0;
	}
    /** initalizes variablies used for loop iteration */
    data_pointer =(uint8_t*) &data_list[curr];
    i = 0;
    j = offset % FILE_SIZE;
    loop = offset/FILE_SIZE;
    while (length != i)
    {
        if (i == num) {
            /* If reading one character too short uncomment */
            //if not good uncomment
            buf[i] = data_pointer[j];
            control_blocks[current_pid].fd_table[fd].file_position += i;
            return i;
        }

	if(control_blocks[current_pid].fd_table[fd].file_position + i >= num){
		return 0;
	}
        /** copies data into buffer */
        buf[i] = data_pointer[j];
        i++;
		j++;

        /** if in first data block and at end, switch to next data block */
        /** if at end of file, switch to next data block */
        if ((j % FILE_SIZE) == 0) {
            j = 0;
            loop += 1;
            data_pointer = (uint8_t*) &data_list[node_list[inode].data_blocks[loop]];
        }
    }
    /** returns number of bytes read and placed in the buffer success */
    control_blocks[current_pid].fd_table[fd].file_position += i;
    return i;
}


/*
 * file_open
 *   DESCRIPTION: opens the file and stores it pcb
 *   INPUTS: const uint8_t * filename
 *   OUTPUTS: none
 *   RETURN VALUE: PCB index on success, -1 on failure
 *   SIDE EFFECTS: none
 */
int32_t file_open(const uint8_t * filename)
{
    int ret;            /** return value */
    int i;              /** loop variable */
    int32_t open;           /** keeps track of open spot on pcb */
    dir_entry_t dentry; /** used to store inodes in pcb */
    fd_block_t fblock; /** file descriptor block to be assigned */
    ret = 0;
    i = 0;
    open = 0;
    /** get the address of the filename */
    ret = read_dentry_by_name(filename, &dentry);

    /** Check the ret */
    if (ret == -1) {
        return ret;
    }
    for (i = 2; i < FDT_SIZE; i++)
    {
        /** the control block is empty, take it */
        if (control_blocks[current_pid].fd_table[i].flags == -1) {
            open = i;
            break;
        }

        /** If there is not anavailable spot, return -1 */
        if (i == FDT_SIZE) {
            return -1;
        }
    }
    fblock.file_operations_pointer = file;
    fblock.inode = dentry.inode_num;
    fblock.file_position = 0;
    fblock.flags = 1;
    // Set flags and file position?
    control_blocks[current_pid].fd_table[open] =  fblock;
    return open;
}

/*
 * file_close
 *   DESCRIPTION: closes a file and removes it from the PCB
 *   INPUTS: const int32_t fd
 *   OUTPUTS: none
 *   RETURN VALUE: 0 on success, -1 on failure
 *   SIDE EFFECTS: none
 */
int32_t file_close(int32_t fd)
{
    /** check for fd validity */
    if (fd < 2 || fd >= FDT_SIZE) {
        return -1;
    }
    if (control_blocks[current_pid].fd_table[fd].flags == -1) {
        return -1;
    }

    /** set the control block entry to -1 */
    control_blocks[current_pid].fd_table[fd].file_operations_pointer = NULL;
    control_blocks[current_pid].fd_table[fd].flags = -1;
    control_blocks[current_pid].fd_table[fd].inode = -1;
    return 0;
}

/*
 * file_write
 *   DESCRIPTION: returns FAILURE as it is a read only filesystem
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: 0 on success, -1 on failure
 *   SIDE EFFECTS: none
 */
int32_t file_write(int32_t fd, const void* buf, int32_t nbytes)
{
    return -1;
}

/*
 * file_read
 *   DESCRIPTION: reads data of file into a buffer
 *   INPUTS: int32_t fd, void* buf, int32_t nbytes
 *   OUTPUTS: none
 *   RETURN VALUE: 0 on success, -1 on failure
 *   SIDE EFFECTS: none
 */
int32_t file_read(int32_t fd, void* buf, int32_t nbytes)
{
    /** calls read data function into the buffer */
    return read_data(fd, control_blocks[current_pid].fd_table[fd].file_position, (uint8_t *)buf, nbytes);
}

/*
 * directory_open
 *   DESCRIPTION: opens the directory file and stores it pcb
 *   INPUTS: const uint8_t * filename
 *   OUTPUTS: none
 *   RETURN VALUE: 0 on success, -1 on failure
 *   SIDE EFFECTS: none
 */
int32_t directory_open(const uint8_t * filename)
{
    int32_t ret;            /** return value */
    int i;              /** loop variable */
    int32_t open;           /** keeps track of open spot on pcb */
    dir_entry_t dentry; /** used to store inodes in pcb */
    fd_block_t  fblock;
    // dentryRead = 0;
    ret = 0;
    i = 0;
    open = 0;
    ret = read_dentry_by_name(filename, &dentry);

    /** check to see if the directory is already open */
    if (ret == -1) {
        return ret;
    }

    /** Search for an open control block and populate it */
    for (i = 2; i < FDT_SIZE; i++)
    {
        if (control_blocks[current_pid].fd_table[i].flags == -1) {
            open = i;
            break;
        }

        if (i == FDT_SIZE - 1) {
            return -1;
        }
    }

    fblock.file_operations_pointer = dir;
    fblock.file_position = 0;
    fblock.inode = 0;
    fblock.flags = casted_block->num_dir_entries;
    control_blocks[current_pid].fd_table[open] = fblock;
    /** return the control block index */
    return open;
}

/*
 * dir_close
 *   DESCRIPTION: Closes the dir and removes it from the filesystem
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: 0 on success, -1 on failure
 *   SIDE EFFECTS: none
 */
int32_t directory_close(int32_t fd)
{

    /** Check for fd validity */
    if (fd < 2 || fd >= FDT_SIZE) {
        return -1;
    }
    if (control_blocks[current_pid].fd_table[fd].flags == -1) {
        return -1;
    }

    /** Clear the control block entry */
    control_blocks[current_pid].fd_table[fd].file_operations_pointer = NULL;
    control_blocks[current_pid].fd_table[fd].flags = -1;
    control_blocks[current_pid].fd_table[fd].inode = -1;
    return 0;
}


/*
 * dir_write
 *   DESCRIPTION: Fails as the directory is readonly
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: 0 on success, -1 on failure
 *   SIDE EFFECTS: none
 */
int32_t directory_write(int32_t fd, const void* buf, int32_t nbytes)
{
    return -1;
}

/*
 * directory_read
 *   DESCRIPTION: reads data of directory into a buffer
 *   INPUTS: int32_t fd, void* buf, int32_t nbytes
 *   OUTPUTS: none
 *   RETURN VALUE: 0 on success, -1 on failure
 *   SIDE EFFECTS: none
 */
int32_t directory_read(int32_t fd, void* buf, int32_t nbytes)
{
    dir_entry_t dentry; /* directory entry variable to read dir entries */
    uint8_t * buffer = (uint8_t *) buf;
    int fnIdx;          // Filename index
    unsigned int bufferIdx = 0;

    while (1)
    {
        if (control_blocks[current_pid].fd_table[fd].flags == 0 || control_blocks[current_pid].fd_table[fd].file_position == MAX_DIR_ENTRIES) {
            return 0;
        }

        read_dentry_by_index(control_blocks[current_pid].fd_table[fd].file_position, &dentry);

        if (dentry.filename[0] == '\0') {
            control_blocks[current_pid].fd_table[fd].file_position++;
        } else {
            /** copies filename into the buffer */
            for (fnIdx = 0; fnIdx < nbytes; fnIdx++)
            {
                if (dentry.filename[fnIdx] == '\0') {
                    buffer[bufferIdx] = (uint8_t)'\0';
                    control_blocks[current_pid].fd_table[fd].file_position++;
                    control_blocks[current_pid].fd_table[fd].flags--;
                    return fnIdx;
                }
                buffer[bufferIdx] = dentry.filename[fnIdx];
                bufferIdx++;
            }
            // If reaches the end.
            if (fnIdx == nbytes) {
                control_blocks[current_pid].fd_table[fd].file_position++;
                control_blocks[current_pid].fd_table[fd].flags--;
                return fnIdx;
            }
        }
    }

    return -1;
}
