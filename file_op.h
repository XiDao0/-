#ifndef _QINIU_LARGE_FILE_OP_H_
#define _QINIU_LARGE_FILE_OP_H_

#include "common.h"

namespace qiniu{
    namespace largefile{
        class FileOperation {
          public:
            FileOperation(const std::string& file_name, const int open_flags = O_RDWR | O_LARGEFILE);
            ~FileOperation();

            int open_file();
            void close_file();

            int flush_file();  // 刷新文件到磁盘

            int unlink_file(); // 从磁盘解除文件

            virtual int pread_file(char *buf, const int32_t nbytes, const int64_t offset); // 按偏移量读取文件内容

            virtual int pwrite_file(const char *buf, const int32_t nbytes, const int64_t offset); // 按偏移量写入文件内容
            int write_file(const char *buf, const int32_t nbytes); // 在当前文件光标位置直接写入

            int64_t get_file_size(); // 获取文件大小

            int ftruncate_file(int64_t length); // 扩容文件大小            
            int seek_file(const int64_t offset); // 文件访问光标位置

            int get_fd() const { return fd_; };

          protected:
            int fd_;
            int open_flags_;
            char *file_name_;

          protected:
            int check_file(); 

          protected:
            static const mode_t OPEN_MODE = 0644; // 文件权限
            static const int MAX_DISK_TIMES = 5; // 磁盘文件最大访问次数
           

        };
    
    }
}


#endif // _QINIU_LARGE_FILE_OP_H_
