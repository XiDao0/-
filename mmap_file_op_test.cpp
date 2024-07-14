#include "mmap_file_op.h"
#include <iostream>

using namespace qiniu;
using namespace std;

int main(void) 
{
    const char *filename = "mmap_file_op.txt";
    largefile::MMapOption mmapoption = {1024, 512, 64};
    largefile::MMapFileOperation *mmfo = new largefile::MMapFileOperation(filename);

    int fd = mmfo->open_file();

    if (fd < 0)
    {
        fprintf(stderr, "file open failed\n");
    }
    






    char buffer[525];

    memset(buffer, '6', 524);

    mmfo->mmap_file(mmapoption);

    int ret = mmfo->pwrite_file(buffer, 524, 0);
    if (ret < 0)
    {
        if (ret == largefile::EXIT_DISK_OPEN_INCOMPLETE)
        {
            fprintf(stderr, "mmfo->pwrite: write length is less than required\n");
        }
        else
        {
            fprintf(stderr, "file %s pwrite failed. resson: %s\n", filename, strerror(-ret));
        }
    }
    
    memset(buffer, '\0', 512);
    
    ret = mmfo->pread_file(buffer, 32, 0);

    cout << ret << ": " << buffer << endl;

    mmfo->munmap_file();

    mmfo->close_file();



    //cout << ret << endl;

//    mmfo->mmap_file_ = new MMapFile(mmapoption);

    

    return 0;
}
