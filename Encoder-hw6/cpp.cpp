#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>


int main()
{
   srand(time(NULL));
    // Path to old and new files
    char newName[100];
    char oldName[20] = "lorem_ipsum.txt";
    int r = (rand()%5)+2;
    for( int i = 0; i < r; ++i){
    newName[i] = 'a' + rand()%20; // starting on '0', ending on '}'
      }
        
      newName[r] = '.';
      newName[r+1] = 't';
      newName[r+2] = 'x';
      newName[r+3] = 't';



    // rename old file with new name
    if (rename(oldName, newName) == 0)
    {
        printf("File renamed successfully.\n");
    }
    else
    {
        printf("Unable to rename files. Please check files exist and you have permissions to modify files.\n");
    }

    return 0;
}