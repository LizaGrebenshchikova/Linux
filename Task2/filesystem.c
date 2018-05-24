/*
This code is used to work with filesystem.
*/
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>

#include "structs.h"
#include "fsfunctions.h"



int main()
{
	int fd = open("filesys", O_RDWR);
	struct Superblock* superblock = (struct Superblock*)malloc(sizeof(struct Superblock));
	pread(fd, superblock, sizeof(struct Superblock), 0);
	struct inode* root = (struct inode*)malloc(sizeof(struct inode));
	unsigned int offset = superblock->start_of_blocks_;
	ipread(superblock, root, fd, offset);

	puts(root->name_of_file_);

	puts("Welcome to my MiniFS! Choose one of the possible commands:\n\
		cd : change directory. EX: cd dir. \n\
		ls : show files and directories in current directory.\n\
		mkdir : create new directory. EX: mkdir new_dir. \n\
		copyfr : copy from outside FS start to end. EX: copyfr start end.\n\
		copyto : copy from start to outside FS end. EX: copyto start end.\n\
		end : stop working.\n\
		");
	//processing the comands.
	char fs_input[120];
	char command[120];
	char arg1[120];
	char arg2[120];
	while(1)
	{
		fgets(fs_input, 120, stdin);

		int counter = -1;
		for(int i = 0; i < strlen(fs_input); ++i)
		{
			if (fs_input[i] == ' ')
			{
				counter = i;
				break;
			}
		}


		if (counter == -1)
		{
			if ((strncmp(fs_input, "end", 3) == 0) && ((strlen(fs_input)) == 4) && (fs_input[3] == '\n'))
			{
				break;
			}
			else if ((strncmp(fs_input, "ls", 2) == 0) && ((strlen(fs_input)) == 3) && (fs_input[2] == '\n'))
			{
			    my_ls(superblock, root, fd);
			}
			else
			{
				puts("Wrong command!");
			}
			continue;
		}

		strncpy(command, fs_input, counter);
		command[counter] = 0;
		int parg1 = -1;

		for(int i = counter + 1; i < strlen(fs_input) ; ++i)
		{
			if (fs_input[i] == ' ')
			{
				parg1 = i;
				arg1[i - counter - 1] = 0;
				break;
			}
		}

		if (parg1 != -1)
		{
			strncpy(arg1, fs_input + counter + 1, (parg1 - counter - 1));
			strncpy(arg2, fs_input + parg1 + 1, strlen(fs_input) - parg1 - 1);
		}

		if (parg1 == -1)
		{
			strncpy(arg1, fs_input + counter + 1, strlen(fs_input) - parg1 - 2);
		}
		arg2[strlen(fs_input) - parg1 - 2] = 0;

		if (strlen(arg1) == 0)
		{
		    puts("empty arg1");
		    continue;
		}

		if (strncmp(command, "cd", 2) == 0)
		{
            my_cd(superblock, root, arg1, fd);
		}


		if (strncmp(command, "mkdir", 5) == 0)
		{
		    my_mkdir(superblock, root, arg1, fd);
		}

		if (strlen(arg2) == 0)
		{
		    puts("empty arg2");
		    continue;
		}

		if (strncmp(command, "copyfr", 6) == 0)
		{
		    my_copyfr(superblock, root, arg1, arg2, fd);
		}

		if (strncmp(command, "copyto", 6) == 0)
		{
		    my_copyto(superblock, root, arg1, arg2, fd);
		}

	}
	pread(fd, superblock, sizeof(struct Superblock), 0);
	free(superblock);
	free(root);
	close(fd);
	return 0;
}
