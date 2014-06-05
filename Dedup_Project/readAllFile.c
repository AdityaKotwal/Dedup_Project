//
//  readAllFile.c
//  Dedup_Project
//
//  Created by Aditya Kotwal on 6/2/14.
//  Copyright (c) 2014 Aditya Kotwal. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <errno.h>
int main(){
    
    DIR* FD;
    struct dirent* in_file;
    FILE    *common_file;
    FILE    *entry_file;
    char    buffer[BUFSIZ];
    
    /* Openiing common file for writing */
    common_file = fopen("/Volumes/TestHD/test/test1.txt", "w");
    if (common_file == NULL)
    {
        fprintf(stderr, "Error : Failed to open common_file - %s\n", strerror(errno));
        return 1;
    }
    
    /* Scanning the in directory */
    if (NULL == (FD = opendir ("/Users/akotwal/Desktop/Dedup_Project")))
    {
        fprintf(stderr, "Error : Failed to open input directory - %s\n", strerror(errno));
        fclose(common_file);
        return 1;
    }
    while ((in_file = readdir(FD)))
    {
        /* On linux/Unix we don't want current and parent directories
         * On windows machine too, thanks Greg Hewgill
         */
        if (!strcmp (in_file->d_name, "."))
            continue;
        if (!strcmp (in_file->d_name, ".."))
            continue;
        /* Open directory entry file for common operation */
        /* TODO : change permissions to meet your need! */
        printf("%s\n",in_file->d_name);
        /*entry_file = fopen(in_file->d_name, "rw");
        if (entry_file == NULL)
        {
            fprintf(stderr, "Error : Failed to open entry file - %s\n", strerror(errno));
            fclose(common_file);
            
            return 1;
        }*/
        
        /* Doing some struf with entry_file : */
        /* For example use fgets */
        /*
        while (fgets(buffer, BUFSIZ, entry_file) != NULL)
        {
   
        }*/
        
        /* When you finish with the file, close it */
        //fclose(entry_file);
    }
    
    /* Don't forget to close common file before leaving */
    fclose(common_file);
    return 0;
}