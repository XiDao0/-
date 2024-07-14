#ifndef _QINIU_LARGEFILE_MMAPFILE_OP_H_
#define _QINIU_LARGEFILE_MMAPFILE_OP_H

#include "common.h"
#include "file_op.h"
#include "mmap_file.h"

namespace qiniu
{
    namespace largefile{
        class MMapFileOperation: public FileOperation
        {
          public:
            MMapFileOperation(const std::string& filename, const int open_flags = O_CREAT | O_RDWR | O_LARGEFILE): 
            FileOperation(filename, open_flags), mmap_file_(NULL), is_mmaped_(false) 
            {
            
            }

            ~MMapFileOperation() 
            {
                if (mmap_file_) 
                {
                    delete(mmap_file_);
                    mmap_file_ = NULL;
                }
            }

            int pread_file(char *buf, const int32_t nbytes, const int64_t offset); 
            int pwrite_file(const char *buf, int32_t nbytes, const int64_t offset);

            int mmap_file(const MMapOption& mmap_opion);
            int munmap_file();

            int flush_file();
            void *get_mmap_data() const; 

          private:
            MMapFile *mmap_file_;
            bool is_mmaped_;
        
        }; 

    }
}

#endif // _QINIU_LARGEFILE_MMAPFILE_OP_H_
