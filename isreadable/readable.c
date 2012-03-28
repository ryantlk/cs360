#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<dirent.h>
#include<string.h>
#include<sys/stat.h>
#include<errno.h>

//max path length of 4096 no real stringent error checking in code to enforce this
#define MAX_PATH_SIZE 4096

void printReadable(char *path);
int isDirectory(char *path);
int isRegular(char *path);

int main(int argc, char *argv[]){
	char* path = (char*)malloc(sizeof(char) * MAX_PATH_SIZE);
	//if no arg given uses pwd to start
	if(argc == 1){
		getcwd(path, MAX_PATH_SIZE);
	}
	else{
		path = argv[1];
	}
	//throwing / on the end if user or cwd doesn't end path with /
	if(path[strlen(path)-1] != '/'){
		strcat(path, "/");
	}
	printReadable(path);
	return 0;
}

void printReadable(char *path){
	DIR *dir;
	struct dirent *dp;
	//if opendir doesnt work print error message
	if((dir = opendir(path)) == NULL){
		fprintf(stderr, "something went horribly wrong: %s\n", strerror(errno));
		exit(1);
	}
	while((dp = readdir(dir)) != NULL){
		//want to ignore . and .. so as not to infinite loop
		if(!(strcmp(dp->d_name, ".") && strcmp(dp->d_name, ".."))){
			continue;
		}
		//setting up path name
		char *newpath = (char*)calloc(sizeof(char),  MAX_PATH_SIZE);
		strcpy(newpath, path);
		strcat(newpath, dp->d_name);
		//if directory adds "/" and recursively calls printReadable
		if(isDirectory(newpath)){
			if(!access(newpath, R_OK | X_OK)){
				strcat(newpath, "/");
				printReadable(newpath);
			}
			continue;
		}
		//prints if file is regular and have read access
		if(isRegular(newpath)){
			if(!access(newpath, R_OK)){
				printf("%s\n", newpath);
			}
		}
		free(newpath);
	}
	closedir(dir);
}

//given a path name returns non zero if path is directory
int isDirectory(char *path){
	struct stat area, *s = &area;
	return (lstat(path, s) == 0) && S_ISDIR(s->st_mode);
}

//given a path name returns non zero if path is a regular file
int isRegular(char *path){
	struct stat area, *s = &area;
	return (lstat(path, s) == 0) && S_ISREG(s->st_mode);
}
