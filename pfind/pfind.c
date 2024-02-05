/*******************************************************************************
* Name        : pfind.c
* Author      : Nicholas Cali & Kyle Henderson
* Date        : 6/10/2021
* Description : finds and prints out files whos permissions match a given string.
* Pledge      : I pledge my honor that I have abided by the Stevens Honor System.
******************************************************************************/
#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <dirent.h>
#include <string.h>

#define BUFSIZE 128

int path = 0;
int perms[] = {S_IRUSR, S_IWUSR, S_IXUSR,
               S_IRGRP, S_IWGRP, S_IXGRP,
               S_IROTH, S_IWOTH, S_IXOTH};


void usage(){
    printf("Usage: ./pfind -d <directory> -p <permissions string> [-h]\n");
}


char* permission_string(struct stat *statbuf) {
    int permission_valid;
    char *result = (char*)malloc(sizeof(char) * 10);
    for (int i = 0; i < 8; i += 3){
        permission_valid = statbuf->st_mode & perms[i];
        if (permission_valid){
            result[i] = 'r';
        }else{
            result[i] = '-';
        }
        permission_valid = statbuf->st_mode & perms[i+1];
        if (permission_valid){
            result[i+1] = 'w';
        }else{
            result[i+1] = '-';
        }
        permission_valid = statbuf->st_mode & perms[i+2];
        if (permission_valid){
            result[i+2] = 'x';
        }else{
            result[i+2] = '-';
        }
    }
    return result;
}


bool baddir(char* parmesan){
    for (int x=0;x<=8;x++){
        if(parmesan[x]!='-'){
            return false;
        }
    }
    return true;
}

bool matcher(char* parmesan, char* other){
    for (int x=0; x<=8;x++){
        if(parmesan[x]!=other[x]){
            return false;
        }
    }
    return true;
}

bool valid(char* parmesan){
    if(strlen(parmesan)!=9){
        return false;
    }
    else{
        for (int x=0;x<8;x+=3){
            if(parmesan[x]!='r'&&parmesan[x]!='-'){
                return false;
            }
            if(parmesan[x+1]!='w'&&parmesan[x+1]!='-'){
                return false;
            }
            if(parmesan[x+2]!='x'&&parmesan[x+2]!='-'){
                return false;
            }
        }
        return true;
    }
}

int runner(char* files, char* parmesan, int path){
    char buf[PATH_MAX];
    DIR *dir;
    if(realpath(files,buf)==NULL){
        fprintf(stderr, "Error: Cannot stat '%s'. %s.\n", files, strerror(errno));
        return EXIT_FAILURE;
    }
    if(!valid(parmesan)){
        fprintf(stderr, "Error: Permissions string '%s' is invalid.\n", parmesan);
        return EXIT_FAILURE;
    }
    if((dir=opendir(buf))==NULL){
        fprintf(stderr, "Error: Cannot open directory '%s'. %s\n", buf, strerror(errno));
        return EXIT_FAILURE;
    }
    struct stat starter;
    char name[PATH_MAX+2];
    struct dirent *begin;
    name[0] = '\0';

    if(strcmp(buf,"/")){
        strncpy(name,buf,PATH_MAX);
    }
    size_t length=strlen(name)+1;
    name[length-1]='/';
    name[length]='\0';

    while ((begin = readdir(dir)) != NULL){
        if (strcmp(begin->d_name, ".") == 0 || strcmp(begin->d_name, "..") == 0){
            continue;
        }
        strncpy(name+length,begin->d_name,PATH_MAX-length);
        if(lstat(begin->d_name, &starter) < 0){
            fprintf(stderr, "Error: Cannot stat file '%s'. %s\n", name, strerror(errno));
            continue;
        }

        struct stat buffer;
        if(stat(name, &buffer) < 0){
            return EXIT_FAILURE;
        }
        char *perm = permission_string(&buffer);
        bool errorchecker = false;
        if (begin->d_type == DT_DIR){
            if(matcher(perm, parmesan)){
                printf("%s\n", name);
                path = 2;
            }
            
            if(baddir(perm)){
                fprintf(stderr, "Error: Cannot open directory '%s. Permission denied.\n", name);
                errorchecker = true;
            }

           if(!errorchecker){
                if(path == 2){
                    path = runner(name, parmesan, 2);
                }else{
                    path = runner(name, parmesan, 0);
                }
            }
        }else{
            if (matcher(perm, parmesan)){
                printf("%s\n", name);
                path =2;
            }
        }
        free(perm);

        
    }
    closedir(dir);
    return path;
}

int main(int argc, char *argv[]) {
    int tracker= 1;
    bool display=false, dir=false, perm=false, misc=false;
    char *directory , *right, *wrong;
    if(argc==1){
        usage();
        return EXIT_FAILURE;
    }else{
        int c;
        while((c= getopt(argc, argv, ":h :d :p"))!=-1){
            switch(c){
            case 'h':
                display=true;
                break;
            case 'd':
                dir=true;
                tracker++;
                if (perm){
                    directory = argv[tracker+1];
                }else{
                    directory = argv[tracker];
                }
                break;
            case 'p':
                perm=true;
                tracker++;
                if (dir){
                    right = argv[tracker+1];
                }else{
                    right = argv[tracker];
                }
                break;
            case '?':
                misc=true;
                wrong = argv[tracker];
            }

        }
        if (misc){
            if(argc==5){
                right = argv[3];
            }else{
                fprintf(stderr, "Error: Unknown option '%s' received.\n", wrong);
                return EXIT_FAILURE;
            }
        }

        if (display){
            usage();
            return EXIT_SUCCESS;
        }
        if(!dir && !perm){
            usage();
            return EXIT_FAILURE;
        }else if(!dir && perm){
            fprintf(stderr, "Error: Required argument -d <directory> not found.\n");
            return EXIT_FAILURE;
        }else if(dir && !perm){
            fprintf(stderr, "Error: Required argument -p <permissions string> not found.\n");
            return EXIT_FAILURE;
        }else{
            if(runner(directory,right,0)!=2){
                    return EXIT_SUCCESS;
            }
            
        }
    }
    return EXIT_SUCCESS;
}
