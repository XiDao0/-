#include "common.h"
#include "file_op.h"

using namespace std;
using namespace qiniu;

int main(void){
    const char *filename = "foptest.txt";

    largefile::FileOperation *fileop = new largefile::FileOperation(filename, O_RDWR | O_LARGEFILE | O_CREAT);

    int fd = fileop->open_file();
    if (fd < 0)
    {
        fprintf(stderr, "open file %s failed. resson: %s\n", strerror(-fd), filename);
        exit(-1);
    }

    char buffer[49];

    memset(buffer, '8', 48);
    int ret = fileop->pwrite_file(buffer, 48, 1024);

    if (ret < 0)
    {
        if (ret == largefile::FILE_DISK_OPEN_INCOMPLETE)
        {
            fprintf(stderr, "pwrite_file: write length is less than required\n");
        }
        else
        {
            fprintf(stderr, "file %s pwrite failed. resson: %s\n", filename, strerror(-ret));
        }
    }

    memset(buffer, 0, 48);
    ret = fileop->pread_file(buffer, 48, 1024);
    if (ret < 0)
    {
        if (ret == largefile::FILE_DISK_OPEN_INCOMPLETE)
        {
            fprintf(stderr, "pwrite_file: write length is less than required\n");
        }
        else 
        {
            fprintf(stderr, "pread file %s failed. resson %s\n", filename, strerror(-ret)); 
        }
    }
    }else {
        buffer[48] = '\0';
        std::cout << buffer << std::endl;
    }

    memset(buffer, '6', 48);
    ret = fileop->write_file(buffer, 48);
    if (ret < 0)
    {
        fprintf(stderr, "file %s write failed. resson: %s\n", filename, strerror(-ret));
    }

    fileop->close_file();
    
    fileop->unlink_file();
    

    return 0;

}
