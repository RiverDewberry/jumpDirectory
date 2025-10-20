#include <stdint.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ncurses.h>
#include <linux/limits.h>
#include <string.h>
#include <dirent.h>
#include <stdlib.h>
#include "../include/bitmap.h"

//adds the string 'add' to the end of 'base' and updates baseLen
#define addToStr(base, baseLen, add) strcpy(base + baseLen, add); baseLen += \
    strlen(add)

#define bold() attron(A_BOLD)
#define unbold() attroff(A_BOLD)

int main(int argc, char **argv)
{
    // NOTE: 'path shorthand' refers to the minimum amount of characters needed
    //to detirmine a file path

    char shortPath[PATH_MAX];//the path shorthand that is entered
    char tempPath[PATH_MAX];//a temporary path str
    char curPath[PATH_MAX];//the path that the shorthand leads to
    
    int spi = 0;//short path index
    int cpi = 0;//current path index
    int ltc = 0;//lines to clear
    int den = 0;//dirs entered
    
    newBitmap(allowedChars, 128);//macro magic
    newBitmap(ACSwap, 8 * sizeof(allowedChars));
    
    //ncurses init
    initscr();
    noecho();
    int in;//user input

    {
        for (int j = strlen(*argv); j-- > 0;)
        {
            if ((*argv)[j] == '/')
            {
                (*argv)[j + 1] = 'd';
                (*argv)[j + 2] = 'i';
                (*argv)[j + 3] = 'r';
                (*argv)[j + 4] = 0;
                break;
            }
        }

        int fd = open(*argv, O_TRUNC | O_WRONLY);
        write(fd, "./", 2);
        close(fd);
    }

    //input loop
    while (true) 
    {
        //if cpi is 0, the user must select between ./ ~/ or / as the starting
        //path
        if (cpi == 0) {
            move(0, 0);
            clrtoeol();
            bold();
            printw("Choose start point: './' '~/' '/'");
            unbold();
            mvprintw(1, 0, "> ");

            while(1)
            {
                in = getch();

                if (in == '.')
                {
                    addToStr(curPath, cpi, "./");
                } else if (in == '~')
                {
                    //hacky work around to get this thing to work
                    addToStr(curPath, cpi, argv[1]);
                    addToStr(curPath, cpi, "/");
                } else if (in == '/')
                {
                    addToStr(curPath, cpi, "/");
                } else {
                    continue;
                }
                
                move(0, 0);
                clrtoeol();
                bold();
                printw("cd %s", curPath);
                unbold();
                printw("\n> ");
                break;        
            }
        }

        //display dirs
        
        struct dirent *dirEnt;
        DIR *dir = opendir(curPath);

        //sets allowed chars to 0
        for (int i = 0; i < sizeof(allowedChars); i++)
        {
            allowedChars[i] = 0;
            ACSwap[i] = 0;
        }
    
        if (dir == NULL)
        {
            printw("!");
            
        } else {

            move(2, 0);

            int dirfn = 0;//dir files number
            int maxflen = 0;//maximum file length in the dir

            while ((dirEnt = readdir(dir)) != NULL)
            {
                dirfn++;
                int l = strlen(dirEnt->d_name) + 2;
                if (l > maxflen) maxflen = l;
            }

            rewinddir(dir);
            strcpy(tempPath, curPath);
            char files[dirfn * maxflen];

            int i = 0;
            while ((dirEnt = readdir(dir)) != NULL)
            {
                struct stat pathStat;
                strcpy(tempPath + cpi, dirEnt->d_name);
                stat(tempPath, &pathStat);

                if(S_ISDIR(pathStat.st_mode) && strcmp(dirEnt->d_name, "."))
                {
                    strcpy(files + (i * maxflen), dirEnt->d_name);
                    files[i * maxflen + strlen(dirEnt->d_name)] = '/';
                    files[1 + i * maxflen + strlen(dirEnt->d_name)] = 0;
                    i++;
                }
            }

            char valid[i];

            for (int j = 0; j < i; j++) valid[j] = 1;

            int matchedChars = 0;

            for (int spc = 0; matchedChars < maxflen && spc < spi;
                    matchedChars++)
            {
                char match = 0;

                for (int k = 0; k < i; k++)
                {
                    if (valid[k] == 0) continue;

                    if (files[k * maxflen + matchedChars] == shortPath[spc])
                    {
                        match = 1;

                        for (int l = 0; l < i; l++)
                        {
                            if (l == k) continue;
                            if (valid[l] == 0) continue;

                            if ((strcmp(
                                        files + (k * maxflen + matchedChars +
                                            1),
                                        files + (l * maxflen + matchedChars +
                                            1)) == 0) && (
                                        files[l * maxflen + matchedChars] ==
                                        shortPath[spc]
                                        ))
                            {
                                match = 0;
                                break;
                            }
                        }
                    } else if (files[k * maxflen + matchedChars] == 0) {
                        valid[k] = 0;
                    }
                }

                if (match) {
                    for (int k = 0; k < i; k++)
                    {
                        if (valid[k] == 0) continue;
                        if (files[k * maxflen + matchedChars] != shortPath[spc])
                            valid[k] = 0;
                    }

                    spc++;
                }
            }

            int sizes[i];
            int newSizes[i];

            char *lastValid;
            char numValid = 0;

            for (int j = 0; j < i; j++)
            {
                if (valid[j] == 0) continue;

                sizes[j] = strlen(files + (j * maxflen));
                newSizes[j] = sizes[j];

                lastValid = files + (j * maxflen);
                numValid++;

                ltc++;
                printw("%s\n", files + (j * maxflen));
            }

            if (numValid && shortPath[spi - 1] == '/')
            {
                addToStr(curPath, cpi, lastValid);

                if (cpi >= sizeof(curPath))
                {
                    endwin();
                    printf("PATH TOO LONG\n");
                    return 0;
                }

                den++;
                spi = 0;
                shortPath[0] = 0;
                move(0, 0);
                clrtoeol();
                bold();
                printw("cd %s", curPath);
                unbold();
                move(1, 0);
                clrtoeol();
                printw("> ");
                continue;
            }

            for (int j = 0; j < i; j++)
            {
                if (!valid[i]) continue;
                for (int k = j; k < i; k++)
                {
                    if (!valid[k]) continue;
                    if (k == j) continue;
                    if (sizes[k] != sizes[j]) continue;

                    for (int l = sizes[j]; l-- > 0;)
                    {
                        if (files[k * maxflen + l] == files[j * maxflen + l])
                        {
                            if (l < newSizes[j]) newSizes[j] = l;
                            if (l < newSizes[k]) newSizes[k] = l;
                        } else {
                            break;
                        }
                    }

                }
            }

            for (int j = 0; j < i; j++)
            {
                if (!valid[j]) continue;
                files[j * maxflen + newSizes[j]] = 0;
            }

            for (int j = matchedChars; j < maxflen; j++)
            {
                for (int k = 0, m = 0; k < i; k++)
                {
                    if (valid[k] == 0) continue;
                    m++;
                    if (valid[k] == -1) continue;

                    if (files[k * maxflen + j] == 0)
                    {
                        valid[k] = -1;
                        continue;
                    }

                    if (getBit(allowedChars, files[k * maxflen + j]) == 0)
                    {
                        bold();
                        mvprintw(m + 1, j, "%c", files[k * maxflen + j]);
                        unbold();
                        setBit(ACSwap, files[k * maxflen + j], 1);
                    } else {
                        mvprintw(m + 1, j, "%c", files[k * maxflen + j]);
                    }
                }


                for (int i = 0; i < sizeof(allowedChars); i++)
                {
                    allowedChars[i] = ACSwap[i];
                }
            }


            closedir(dir);
        }

        move(1, 2 + spi);
        
        //get user input
        
        while (1)
        {
            in = getch();

            //if backspace is pressed
            if (in == '\b' || in == 127) {
                
                //deletes form shortpath if there is room
                if (spi != 0) {
                    spi--;
                    shortPath[spi] = 0;
                    mvprintw(1, getcurx(stdscr) - 1, " ");
                    move(1, getcurx(stdscr) - 1);
                } else {

                    if (den != 0)
                    {
                        for (int j = strlen(curPath) - 1; j-- > 0;)
                        {
                            if (curPath[j] == '/')
                            {
                                curPath[j + 1] = 0;
                                cpi = j + 1;
                                break;
                            }
                        }

                        move(0, 0);
                        clrtoeol();
                        bold();
                        printw("cd %s", curPath);
                        unbold();
                        move(1, 0);
                        printw("> ");
                        den--;
                    }  else {
                        cpi = 0;
                        clrtoeol();
                    }
                }
                break;

            }

            if (in == '\n')
            {
                endwin();

                int fd = open(*argv, O_TRUNC | O_WRONLY);
                write(fd, curPath, cpi);
                close(fd);

                return 0;
            }

            //if a non-char was pressed and not used
            if (in > 127) {
                continue;
            }

            //breaks when valid input
            if (getBit(allowedChars, in)) {
                //adds char to shortPath
                shortPath[spi] = in;
                spi++;
                shortPath[spi] = 0;

                //prints shortPath
                mvprintw(1, 0, "> %s", shortPath);
                break;
            }
        }

        move(2, 0);
        for (int i = 0; i < ltc; i++)
        {
            move(i + 2, 0);
            clrtoeol();
        }
        ltc = 0;

    }

    //ncurses un-init
    endwin();
}
