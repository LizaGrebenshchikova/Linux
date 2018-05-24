/*
structs of superblock and inode, initialization of them, function for creation directory ( as it used both in filesystem.c and create_system.c), function for cheaking the block( free or occupied),
function to find free block .
*/

#ifndef STRUCTS_H
#define STRUCTS_H

#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

struct Superblock
{
	unsigned int num_of_blocks_;
	unsigned int num_of_free_blocks_;

	unsigned int size_of_block_;
	unsigned int size_of_inode_;

	unsigned int start_of_blocks_; // the beginning of blocks in the big file.

};

struct Superblock* initialization_superblock( unsigned int num_of_blocks, unsigned int num_of_free_blocks,
												unsigned int size_of_block, unsigned int size_of_inode, unsigned int start_of_blocks)
{
	struct Superblock* superblock = (struct Superblock*)malloc(sizeof(struct Superblock));

	superblock->num_of_blocks_ = num_of_blocks;
	superblock->num_of_free_blocks_ = num_of_free_blocks;

	superblock->size_of_block_ = size_of_block;
	superblock->size_of_inode_ = size_of_inode;

	superblock->start_of_blocks_ = start_of_blocks;
	return superblock;
}

struct inode
{
	unsigned int size_of_file_;
	time_t date_of_creation_;

	unsigned int parent_;
	int blocks_[12];
	int add_blocks_; //reference to the blocks, which could be used for saving the file.

	char name_of_file_[60];
	char type_; // directory or file
	int num_;
};

struct inode* initialization_inode (unsigned int parent, const char* name_of_file, char type, int num)
{
	struct inode* ind = (struct inode*)malloc(sizeof(struct inode));
	ind->parent_ = parent;
	strncpy(ind->name_of_file_, name_of_file, strlen(ind->name_of_file_));
	ind->type_ = type;
	ind->num_ = num;
	time(&(ind->date_of_creation_));
	return ind;
}

void ipwrite(struct Superblock* superblock, struct inode* ind, int fd, int offset)
{
	pwrite(fd, &ind->size_of_file_, sizeof(int), offset);
	offset += sizeof(int);
	pwrite(fd, &ind->date_of_creation_, sizeof(time_t), offset);
    offset += sizeof(time_t);
    pwrite(fd, &ind->parent_ , sizeof(int), offset);
    offset += sizeof(int);
    pwrite(fd, ind->blocks_, 12 * sizeof(int), offset);
    offset += 12 * sizeof(int);
	pwrite(fd, &ind->add_blocks_, sizeof(int), offset);
    offset += sizeof(int);
	pwrite(fd, ind->name_of_file_, 60, offset);
    offset += 60;
    pwrite(fd, &ind->type_, 1, offset);
    offset++;
	pwrite(fd, &ind->num_, sizeof(int), offset);
}

char checking_block(int fd, int necessary_block)
{
	int tmp = (necessary_block / 8);
	int offset = sizeof(struct Superblock) + tmp;
	unsigned char buf;
	int read = pread(fd, &buf, 1, offset);
	int rest = tmp % 8;
	unsigned char bit = buf & (1 << (8 - rest - 1 ));
	return bit;
}

int find_num_free_block(int fd, struct Superblock* superblock)
{
	if (superblock->num_of_free_blocks_ == 0)
	{
		printf("%s\n", "No empty space!");
		return -1;
	}
	int bytes = superblock->num_of_blocks_ / 8;
	int num_of_free_block;
	char flag = 0;
	int rest;
	for (int i = 0; i < bytes; ++i)
	{
		unsigned char buf;
		int offset = sizeof(struct Superblock) + i;
		int read = pread(fd, &buf, 1, offset);
		unsigned char c = 128;
		for(int j = 0; j < 8; ++j)
		{
			if ((buf & c) == 0)
			{
				rest = j;
				flag = 1;
				break;
			}
			c >>= 1;
		}
		if (flag)
		{
			num_of_free_block = i * 8 + rest;
			return num_of_free_block;
		}
	}
}

void change_block_occup(struct Superblock* superblock, int fd, int num_of_block)
{
	int offset = sizeof(struct Superblock) + (num_of_block / 8);
	int  rest = num_of_block % 8;
	unsigned char buf;
	int read = pread(fd, &buf, 1, offset);

	unsigned char new_byte;
	new_byte = buf | (1 << (8 - rest - 1));
	int write = pwrite(fd, &new_byte, 1, offset);
	superblock->num_of_blocks_--;
	return;
}

void change_block_free(struct Superblock* superblock, int fd, int num_of_block)
{
	int offset = sizeof(struct Superblock) + (num_of_block / 8);
	int  rest = num_of_block % 8;
	unsigned char buf;
	int read = pread(fd, &buf, 1, offset);

	unsigned char new_byte;
	new_byte = buf & (~(1 << (8 - rest - 1)));
	int write = pwrite(fd, &new_byte, 1, offset);
	superblock->num_of_blocks_++;
	return;
}

void creation_directory(int fd, struct Superblock* superblock, unsigned int parent, const char* name_of_file)
{
	int free_num = find_num_free_block(fd, superblock);
	change_block_occup(superblock, fd, free_num);
	struct inode* ind = initialization_inode(parent, name_of_file, 'd', free_num);
	ipwrite(superblock, ind, fd, free_num * superblock->size_of_block_);
	free(ind);
	return;
}

void ipread(struct Superblock* superblock, struct inode* ind, int fd, int offset)
{
	pread(fd, &ind->size_of_file_, sizeof(int), offset);
	offset += sizeof(int);
	pread(fd, &ind->date_of_creation_, sizeof(time_t), offset);
    offset += sizeof(time_t);
    pread(fd, &ind->parent_ , sizeof(int), offset);
    offset += sizeof(int);
    pread(fd, ind->blocks_, 12 * sizeof(int), offset);
    offset += 12 * sizeof(int);
	pread(fd, &ind->add_blocks_, sizeof(int), offset);
    offset += sizeof(int);
	pread(fd, ind->name_of_file_, 60, offset);
    offset += 60;
    pread(fd, &ind->type_, 1, offset);
    offset++;
	pread(fd, &ind->num_, sizeof(int), offset);
}

#endif //STRUCTS_H
