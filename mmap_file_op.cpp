#include "mmap_file_op.h"
#include <stdio.h>
#include "common.h"

static int debug = 1;

namespace qiniu{
    namespace largefile {
        int MMapFileOperation::mmap_file(const MMapOption& mmap_option)
        {
            if (mmap_option.max_mmap_size_ < mmap_option.first_mmap_size_ || mmap_option.max_mmap_size_ <= 0)
            {
                return TFS_ERROR;
            }

            int fd = FileOperation::check_file();
            if (fd < 0)
            {
                fprintf(stderr, "MMapFileOperation::mmap_file - checked file error\n");
                return TFS_ERROR;
            }
            
            if (!is_mmaped_)
            {
                if(mmap_file_)
                {
                    delete(mmap_file_);
                }
                mmap_file_ = new MMapFile(mmap_option, fd);
                is_mmaped_ = mmap_file_->map_file(true);
            }

            if (!is_mmaped_)
            {
                return TFS_ERROR;
            }

            return TFS_SUCCESS;
        }

        int MMapFileOperation::munmap_file()
        {
            if (is_mmaped_ && mmap_file_ != NULL)
            {
                delete(mmap_file_);
                is_mmaped_ = false;
            }

            return TFS_SUCCESS;
        }

        void* MMapFileOperation::get_mmap_data() const 
        {
            if (is_mmaped_)
            {
                return mmap_file_->get_data();
            }

            return NULL;
        }

        int MMapFileOperation::pread_file(char *buf, const int32_t nbytes, const int64_t offset)
        {
            if (is_mmaped_ && (nbytes + offset) > mmap_file_->get_size())
            {
                if (debug)
                {   
                    fprintf(stdout, "MMapFileOperation:pread_file size:%d, offset %" PRIu64/*__PRI64_PREFIX*/ "d, map_file size:%d, remap once(Ps:mmap size insufficeien)\n",
                        nbytes, offset, mmap_file_->get_size());
                    mmap_file_->remap_file();
                }
            }

            if (is_mmaped_ && (nbytes + offset) <= mmap_file_->get_size())
            {
                memcpy(buf,(char *)mmap_file_->get_data()+offset, nbytes);
                return TFS_SUCCESS;
            }

            return FileOperation::pread_file(buf, nbytes, offset);
        }

        int MMapFileOperation::pwrite_file(const char *buf, const int32_t size, const int64_t offset)
        {   // 有映射
            if (is_mmaped_ && (size + offset) > mmap_file_->get_size())
            {
                if (debug)
                {
                    fprintf(stdout, "MMapFileOperation:pwrite_file size:%d, offset %" PRIu64 "d, map_file size:%d,"
                    "remap once(Ps:mmap size insufficeien)\n", 
                        size, offset, mmap_file_->get_size());
                    mmap_file_->remap_file();
                }
            }

            if (is_mmaped_ && (size + offset) <= mmap_file_->get_size())
            {
                memcpy((char *)mmap_file_->get_data()+offset, buf, size);
                return TFS_SUCCESS;
            }
            // 情况2 无映射
            return FileOperation::pwrite_file(buf, size, offset);
        }

        int MMapFileOperation::flush_file()
        {
            if (is_mmaped_)
            {
                if (mmap_file_->sync_file()) 
                {
                    return TFS_SUCCESS;
                }
                else 
                {
                    return TFS_ERROR;    
                }
            }

            return FileOperation::flush_file();
        }
    }
}
