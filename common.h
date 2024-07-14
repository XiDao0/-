#ifndef _COMMON_H_INCLUDED_
#define _COMMON_H_INCLUDED_


#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdlib.h>
#include <inttypes.h>
#include <cassert>
#include <sstream>

namespace qiniu
{
    namespace largefile
    {
        const int32_t TFS_ERROR = -1;
        const int32_t TFS_SUCCESS = 0;
        const int32_t EXIT_DISK_OPEN_INCOMPLETE = -8012; // read or write length is less than required
        const int32_t EXIT_INDEX_ALREADY_LOADED_ERROR = -8013; // index is already loaded when create or load
        const int32_t EXIT_META_UNEXPECT_FOUND_ERROR = -8014;
        const int32_t EXIT_INDEX_CORRUPT_ERROR = -8015;
        const int32_t EXIT_BLOCKID_CONFLICT_ERROR = -8016;
        const int32_t EXIT_BUCKET_CONFIGURE_ERROR = -8017;
        const int32_t EXIT_META_NOT_FOUND_ERROR = -8018;
        const int32_t EXIT_BLOCKID_ZERO_ERROR = -8019;

        static const std::string MAINBLOCK_DIR_PREFIX = "/mainblock/";
        static const std::string INDEX_DIR_PREFIX = "/index/";
        static const mode_t DIR_MODE = 0755;

		struct MMapOption  // 命名
		{
			int32_t max_mmap_size_;
			int32_t first_mmap_size_;
			int32_t per_mmap_size_;
			
		};

        enum OperType {
            C_OPER_INSERT = 1,
            C_OPER_DELETE
        };

        struct BlockInfo // 块信息
        {
            uint32_t block_id_;         // 块id信息（编号）
            int32_t  version_;          // 当前版本号
            int32_t  file_count_;       // 已保存文件数量
            int32_t  size_;             // 已保存文件数据总大小
            int32_t  del_file_count_;   // 已删除文件数量
            int32_t  del_size_;         // 已删除文件数量总大小
            uint32_t seq_no_;           // 下一个可分配的文件编号
            
            BlockInfo() 
            {
                memset(this, 0, sizeof(BlockInfo));
            }
        
            inline bool operator==(const BlockInfo &info) const
            {
                return block_id_ == info.block_id_ && version_ == info.version_ && file_count_ == info.file_count_ && size_ == info.size_ 
                    && del_file_count_ == info.del_file_count_ && del_size_ == info.del_size_ && seq_no_ == info.seq_no_;
            }
        };

        struct MetaInfo
        {
          public:
            MetaInfo()
            {
                init();
            }
            MetaInfo(const uint64_t file_id, const int32_t in_offset, const int32_t size, const int32_t next_meta_offset)
            {
                fileid_ = file_id;
                location_.inner_offset_ = in_offset;
                location_.size_ = size;
                next_meta_offset_ = next_meta_offset;
            }

            MetaInfo(const MetaInfo& meta_info)
            {
                memcpy(this, static_cast<const void*>(&meta_info), sizeof(meta_info));
            }

            bool operator==(const MetaInfo& meta_info) const
            {
                return fileid_ == meta_info.fileid_ && location_.inner_offset_ == meta_info.location_.inner_offset_ && 
                    location_.size_ == meta_info.location_.size_ && next_meta_offset_ == meta_info.next_meta_offset_;
            }

            MetaInfo& operator=(const MetaInfo& meta_info)
            {
                if (this == &meta_info)
                {
                    return *this;
                }
            
                fileid_ = meta_info.fileid_;
                location_.inner_offset_ = meta_info.location_.inner_offset_;
                location_.size_ = meta_info.location_.size_;
                next_meta_offset_ = meta_info.next_meta_offset_;
                return *this;
            }

            MetaInfo& clone(const MetaInfo& meta_info)
            {
                assert(!(this == &meta_info));

                fileid_ = meta_info.fileid_;
                location_.inner_offset_ = meta_info.location_.inner_offset_;
                location_.size_ = meta_info.location_.size_;
                next_meta_offset_ = meta_info.next_meta_offset_;
                return *this;
            }

            uint64_t get_key() const
            {
                return fileid_;
            }

            void set_key(const uint64_t key)
            {
                fileid_ = key;
            }

            int64_t get_file_id() const
            {
                return fileid_;
            }

            void set_file_id(const uint64_t file_id)
            {
                fileid_ = file_id;
            }

            int32_t get_inner_offset() const
            {
                return location_.inner_offset_;
            }

            void set_inner_offset(const int32_t in_offset)
            {
                location_.inner_offset_ = in_offset;
            }

            int32_t get_file_size() const
            {
                return location_.size_;
            }
    
            void set_file_size(const int32_t file_size)
            {
                location_.size_ = file_size;
            }

            int32_t get_next_meta_offset() const
            {
                return next_meta_offset_;
            }

            void set_next_meta_offset(const int32_t offset) const 
            {
                next_meta_offset_ = offset;
            }

          private:

            uint64_t fileid_;

            struct Location{
                int32_t inner_offset_;
                int32_t size_;
            }location_;

            mutable int32_t next_meta_offset_;

          private:
               void init()
                {
                    fileid_ = 0;
                    location_.inner_offset_ = 0;
                    location_.size_ = 0;
                    next_meta_offset_ = 0;
                }
        };
    }
}


#endif /* _COMMON_H_INCLUDED_ */
