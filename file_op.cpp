#include "file_op.h"
#include <stdio.h>
#include "common.h"

namespace qiniu
{
    namespace largefile
    {
        FileOperation::FileOperation(const std::string &file_name, const int open_flags) :
        fd_(-1), open_flags_(open_flags)
        {
            file_name_ = strdup(file_name.c_str());
        }

        FileOperation::~FileOperation()
        {
            if (fd_ > 0 )
            {
                ::close(fd_);   // 全局  i
                fd_ = -1;
            }

            if (NULL != file_name_)
            {
                free(file_name_);
                file_name_ = NULL;
            }
        }

        int FileOperation::open_file()
        {
            if (fd_ > 0)
            {
                ::close(fd_);
                fd_ = -1;
            }

            fd_ = ::open(file_name_, open_flags_, OPEN_MODE);

            if (fd_ < 0) 
            {
                return -errno;
            }

            return fd_;
        }

        void FileOperation::close_file()
        {
            if (fd_ < 0)
            {
                return;
            }

            ::close(fd_);
            fd_ = -1;
        }

        int64_t FileOperation::get_file_size()
        {
            int fd = check_file();

            if(fd < 0) 
            {
                return -1;
            }

            struct stat statbuf;
            if ( fstat(fd, &statbuf) != 0)
            {
                return -1;
            }
            return statbuf.st_size;
        }

        int FileOperation::check_file()
        {
            if (fd_ < 0)
            {
                fd_ = open_file();
            }

            return fd_;
        }

        int FileOperation::ftruncate_file(int64_t length)
        {
            int fd = check_file();

            if(fd < 0)
            {
                return fd;                
            }   
            
            return ftruncate(fd, length);
        }

        int FileOperation::seek_file(const int64_t offset)
        {
            int fd = check_file();

            if (fd < 0 )
            {
                return fd;
            }

            return lseek(fd, offset, SEEK_SET);
        }

        int FileOperation::flush_file()
        {
            if ((open_flags_ & O_SYNC))
            {
                return 0;
            }

            int fd = check_file();

            if (fd < 0)
            {
                return fd;
            }

            return fsync(fd);
        }

        int FileOperation::unlink_file()
        {
            close_file();

            return unlink(file_name_);
        }

        int FileOperation::pread_file(char *buf, const int32_t nbytes, const int64_t offset)
        {
            int32_t left = nbytes;          // 文件读取剩余
            int64_t read_offset = offset ;   // 文件读取偏移量
            int32_t read_len = 0;           // 文件读取长度
            char *p_tmp = buf;

            int i = 0;

            while (left > 0)
            {
                ++i;

                if (i > MAX_DISK_TIMES) { break; }  // 判断读取磁盘次数

                if (check_file() < 0) { return -errno; }    // 检查d文件是否打开

                read_len = ::pread(fd_, buf, left, read_offset); // 使用系统函数读取文件内容

                if (read_len < 0)
                {
                    read_len = -errno;      // 及时获取出错日志

                    if (-read_len == EINTR || EAGAIN == -read_len) // 判断错误日志是否重读一次或再读一次
                    {                                              // 出错内容判断
                        continue;
                    }else if (EBADF == -read_len){
                        fd_ = -1;
                        continue;
                    }else {
                        return read_len;
                    }
                }else if (0 == read_len) { break; }
                
                left -= read_len;
                p_tmp += read_len;
                read_offset += read_len;
            }
            if (0 != left)
            {
                return EXIT_DISK_OPEN_INCOMPLETE;
            }

            return TFS_SUCCESS;

        }

        int FileOperation::pwrite_file(const char *buf, const int32_t nbytes, const int64_t offset)
        {
            int32_t left = nbytes;          // 文件读取剩余
            int64_t write_offset = offset;   // 文件读取偏移量
            int32_t written_len = 0;           // 文件读取长度
            const char *p_tmp = buf;

            int i = 0;

            while (left > 0)
            {
                ++i;

                if (i > MAX_DISK_TIMES) { break; }  // 判断写入磁盘次数

                if (check_file() < 0) { return -errno; }    // 检查d文件是否打开

                written_len = ::pwrite(fd_, buf, left, write_offset); // 使用系统函数读取文件内容

                if (written_len < 0)
                {
                    written_len = -errno;      // 及时获取出错日志

                    if (-written_len == EINTR || EAGAIN == -written_len) // 判断错误日志是否重读一次或再读一次
                    {                                              // 出错内容判断
                        continue;
                    }else if (EBADF == -written_len){
                        fd_ = -1;
                        continue;
                    }else {
                        return written_len;
                    }
                }
                left -= written_len;
                p_tmp += written_len;
                write_offset += written_len;
            }
            if (0 != left)
            {
                return EXIT_DISK_OPEN_INCOMPLETE;
            }

            return TFS_SUCCESS;
        }

        int FileOperation::write_file(const char *buf, const int32_t nbytes)
        {
            int32_t left = nbytes;          // 文件读取剩余
            int32_t written_len = 0;           // 文件读取长度
            const char *p_tmp = buf;

            int i = 0;

            while (left > 0)
            {
                ++i;

                if (i > MAX_DISK_TIMES) { break; }  // 判断读取磁盘次数

                if (check_file() < 0) { return -errno; }    // 检查d文件是否打开

                written_len = ::write(fd_, buf, left); // 使用系统函数写入文件内容

                if (written_len < 0)
                {
                    written_len = -errno;      // 及时获取出错日志

                    if (-written_len == EINTR || EAGAIN == -written_len) // 判断错误日志是否重读一次或再读一次
                    {                                              // 出错内容判断
                        continue;
                    }else if (EBADF == -written_len){
                        fd_ = -1;
                        continue;
                    }else {
                        return written_len;
                    }
                }                
                left -= written_len;
                p_tmp += written_len;
            }
            if (0 != left)
            {
                return EXIT_DISK_OPEN_INCOMPLETE;
            }

            return TFS_SUCCESS;
        }



    }
}
