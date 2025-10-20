#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

int main(int argc, char **argv)
{
    int strlen = 0;

    for (int j = 0;; j++)
    {
        if ((*argv)[j] == 0)
        {
            break;
        } else {
            strlen++;
        }
    }

    for (int j = strlen; j-- > 0;)
    {
        if ((*argv)[j] == '/')
        {
            (*argv)[j] = 0;
            break;
        }
    }

    chdir(*argv);

    int fd = open("./dir", O_RDONLY);

    struct stat statRet;
    fstat(fd, &statRet);

    char buf[statRet.st_size];

    read(fd, buf, statRet.st_size);
    write(1, buf, statRet.st_size);

    return 0;
}
