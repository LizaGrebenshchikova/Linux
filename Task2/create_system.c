/*
In the big file (filesys) we will have superblock (where we have all information in our filesystem), bitmap (for blocks, 0 - is free, 1 - is occupied), blocks, where we will save files, inodes.
This code is used to create filesystem: create filesys, create superblock and write it to filesys, write to filesys bitmap, create root directory. Have to launch it firstly.
*/
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>

#include "structs.h"

int main()
{
	int fd = open("filesys", O_CREAT | O_RDWR, 0666);
	//create superblock.
	struct Superblock* superblock;
	unsigned int num_of_blocks = 65536;
	unsigned int num_of_free_blocks = 65536;
	unsigned int size_of_block = 256;// size of inode.
	unsigned int size_of_inode = sizeof(struct inode);
	int bitmap_size = num_of_blocks / 8;
	unsigned int start_of_blocks = (bitmap_size / size_of_block + 1) * size_of_block;
	superblock = initialization_superblock( num_of_blocks, num_of_free_blocks, size_of_block, size_of_inode, start_of_blocks);
	// add superblock to filesys.
	write(fd, superblock, sizeof(struct Superblock));


	//add bitmap to filesys.
	int tmp = sizeof(superblock) + bitmap_size;
	int num_of_1_bm_start = (tmp + size_of_block - 1) / size_of_block; // number of 1, that we have to write in the file (blocks for superblock and bitmap).
	superblock->num_of_free_blocks_ -= num_of_1_bm_start;

	char st = 255;
	int len_of_cycle = (num_of_1_bm_start / 8);
	for( int i = 0; i < len_of_cycle; ++i)
	{
		write(fd, &st, sizeof(char));
	}

	int rest = num_of_1_bm_start % 8;
	st = (255 >> (8 - rest)) << (8 - rest);
	write(fd, &st, sizeof(char));

	st = 0;
	for (int i = 0; i < (bitmap_size - len_of_cycle - 1); ++i)
	{
		write(fd, &st, sizeof(char));
	}

	// create root directory.
	unsigned int parent = 0;
	const char* name_of_file = "r";
	creation_directory(fd, superblock, parent, name_of_file);
	free(superblock);

	close(fd);
	return 0;
}
