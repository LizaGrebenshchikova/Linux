#ifndef FSFUNCTIONS_H
#define FSFUNCTIONS_H
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <time.h>

#include "structs.h"

int find_first_empty_index(struct Superblock* sb, struct inode* node, int fd) {

    int index;
    char flag = 0;
    for (int i = 0; i < 12; ++i)
    {
        if (node->blocks_[i] == 0)
        {
            flag = 1;
            index = i;
            break;
        }
    }

    if (flag == 0 && node->add_blocks_ != 0)
    {
        int* add_blocks = (int*)malloc(sb->size_of_block_);
        pread(fd, add_blocks, sb->size_of_block_, node->add_blocks_ * sb->size_of_block_);
        for (int i = 0; i < sb->size_of_block_ / sizeof(int); ++i)
        {
            if (add_blocks[i] == 0)
            {
                flag = 1;
                index = i + 12;
                break;
            }
        }
        free(add_blocks);
    }

    if (flag == 0)
        index = -1;
    return index;
}

int find_index_by_name(struct Superblock* sb, struct inode* node, const char* name, int fd) {

    int index;
    char flag = 0;
    struct inode* ind = initialization_inode(0, "", '-', 0);
    for (int i = 0; i < 12; ++i)
    {
        if (node->blocks_[i] != 0)
        {
            ipread(sb, ind, fd, node->blocks_[i] * sb->size_of_block_);
            if (strncmp(ind->name_of_file_, name, strlen(name)))
            {
                flag = 1;
                index = i;
                break;
            }
        }
    }

    if (flag == 0 && node->add_blocks_ != 0)
    {
        int* add_blocks = (int*)malloc(sb->size_of_block_);
        pread(fd, add_blocks, sb->size_of_block_, node->add_blocks_ * sb->size_of_block_);
        for (int i = 0; i < sb->size_of_block_ / sizeof(int); ++i)
        {
            if (add_blocks[i] != 0)
            {
                ipread(sb, ind, fd, add_blocks[i] * sb->size_of_block_);
                if (strncmp(ind->name_of_file_, name, strlen(name)) && strlen(name) == strlen(ind->name_of_file_))
                {
                    flag = 1;
                    index = i;
                    break;
                }
            }
        }
        free(add_blocks);
    }

    if (flag == 0)
        index = -1;
    return index;
}

void add_child_inode(struct Superblock* sb, struct inode* node, int num, int fd) {

    int index = find_first_empty_index(sb, node, fd);
    if (index == -1 && node->add_blocks_ != 0)
        return;
    if (index == -1)
        index = 12;

    if (index < 12)
    {
        node->blocks_[index] = num;
    }
    else
    {
        if (node->add_blocks_ == 0)
        {
            int free_num = find_num_free_block(fd, sb);
            change_block_occup(sb, fd, free_num);
            node->add_blocks_ = free_num;
            index = 0;
        }
        else
        {
            index -= 12;
        }
        pwrite(fd, &num, sizeof(int), node->add_blocks_ * sb->size_of_block_ + index * sizeof(int));
    }
}

void my_mkdir(struct Superblock* sb, struct inode* node, const char* name, int fd) {

    int index = find_index_by_name(sb, node, name, fd);
    if (index != -1)
        return;

    int free_num = find_num_free_block(fd, sb);
    change_block_occup(sb, fd, free_num);

    struct inode* new_node = initialization_inode(node->num_, name, 'd', free_num);

    add_child_inode(sb, node, free_num, fd);

    ipwrite(sb, new_node, fd, free_num * sb->size_of_block_);
    ipwrite(sb, node, fd, node->num_ * sb->size_of_block_);

    free(new_node);
}

void my_cd(struct Superblock* sb, struct inode* node, const char* name, int fd) {

    if (strncmp(name, "..", 2))
    {
        if (node->parent_ == 0)
            return;

        struct inode* parent = initialization_inode(0, "", '-', 0);
        ipread(sb, parent, fd, node->parent_ * sb->size_of_block_);

        struct inode* tmp = node;
        node = parent;
        parent = tmp;
        free(parent);
        return;
    }

    struct inode* ind = initialization_inode(0, "", '-', 0);
    int index = find_index_by_name(sb, node, name, fd);
    if (index == -1)
    {
        free(ind);
        return;
    }
    else
        ipread(sb, ind, fd, node->blocks_[index] * sb->size_of_block_);

    if (ind->type_ != 'd')
    {
        puts("Not a directory!");
    }
    else
    {
        struct inode* tmp = node;
        node = ind;
        ind = tmp;
    }
    free(ind);
}

void my_ls(struct Superblock* sb, struct inode* node, int fd) {

    struct inode* ind = initialization_inode(0, "", '-', 0);
    struct tm* date;
    for (int i = 0; i < 12; ++i)
    {
        if (node->blocks_[i] != 0)
        {
            ipread(sb, ind, fd, node->blocks_[i] * sb->size_of_block_);
            date = localtime(&ind->date_of_creation_);
            printf("%c %s %d %s", ind->type_, ind->name_of_file_, ind->size_of_file_, asctime(date));
        }
    }

    if (node->add_blocks_ != 0)
    {
        int* add_blocks = (int*)malloc(sb->size_of_block_);
        pread(fd, add_blocks, sb->size_of_block_, node->add_blocks_ * sb->size_of_block_);
        for (int i = 0; i < sb->size_of_block_ / sizeof(int); ++i)
        {
            if (add_blocks[i] != 0)
            {
                ipread(sb, ind, fd, add_blocks[i] * sb->size_of_block_);
                date = localtime(&ind->date_of_creation_);
                printf("%c %s %d %s", ind->type_, ind->name_of_file_, ind->size_of_file_, asctime(date));
            }
        }
        free(add_blocks);
    }
    free(ind);
}

void my_copyfr(struct Superblock* sb, struct inode* node, const char* namefr, const char* name, int fd) {

    int fdfr = open(namefr, O_RDONLY);

    struct stat st;
    fstat(fdfr, &st);
    int free_block = find_num_free_block(fd, sb);
    change_block_occup(sb, fd, free_block);
    struct inode* new_node = initialization_inode(node->num_, name, '-', free_block);
    new_node->size_of_file_ = st.st_size;
    add_child_inode(sb, node, free_block, fd);

    char* block = (char*)malloc(sb->size_of_block_);
    int bytes_read = read(fdfr, block, sb->size_of_block_);
    int free_num;
    while (bytes_read > 0)
    {
        free_num = find_num_free_block(fd, sb);
        change_block_occup(sb, fd, free_num);
        pwrite(fd, block, bytes_read, free_num * sb->size_of_block_);
        add_child_inode(sb, new_node, free_num, fd);
        ipwrite(sb, new_node, fd, free_block * sb->size_of_block_);
        bytes_read = read(fdfr, block, sb->size_of_block_);
    }

    ipwrite(sb, new_node, fd, free_block * sb->size_of_block_);
    ipwrite(sb, node, fd, node->num_ * sb->size_of_block_);
    close(fdfr);
    free(new_node);
    free(block);
}

void my_copyto(struct Superblock* sb, struct inode* node, const char* namefr, const char* name, int fd) {

    struct inode* ind = initialization_inode(0, "", '-', 0);
    int index = find_index_by_name(sb, node, namefr, fd);
    if (index == -1)
        return;

    ipread(sb, ind, fd, node->blocks_[index] * sb->size_of_block_);
    int fdto = open(name, O_CREAT | O_WRONLY, 0666);
    char* block = (char*)malloc(sb->size_of_block_);
    int size = ind->size_of_file_;
    for (int i = 0; i < 12; ++i)
    {
        if (ind->blocks_[i] != 0)
        {
            pread(fd, block, sb->size_of_block_, ind->blocks_[i] * sb->size_of_block_);

            puts(block);
            if (size < sb->size_of_block_)
            {
                write(fdto, block, size);
            }
            else
            {
                write(fdto, block, sb->size_of_block_);
            }
            size -= sb->size_of_block_;
        }
    }

    if (size > 0)
    {
        int* add_blocks = (int*)malloc(sb->size_of_block_);
        pread(fd, add_blocks, sb->size_of_block_, ind->add_blocks_ * sb->size_of_block_);

        for (int i = 0; i < sb->size_of_block_ / sizeof(int); ++i)
        {
            if (add_blocks[i] != 0)
            {
                pread(fd, block, sb->size_of_block_, add_blocks[i] * sb->size_of_block_);
                if (size < sb->size_of_block_)
                {
                    write(fdto, block, size);
                }
                else
                {
                    write(fdto, block, sb->size_of_block_);
                }
                size-= sb->size_of_block_;
            }
        }
        free(add_blocks);
    }

    free(ind);
    free(block);
    close(fdto);
}

#endif //FSFUNCTIONS_H
