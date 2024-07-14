#ifndef _QINIU_LARGEFILE_INDEX_HANDLE_H_
#define _QINIU_LARGEFILE_INDEX_HANDLE_H_

#include "mmap_file_op.h" 
#include "common.h"

namespace qiniu {
    namespace largefile
    {
        struct IndexHeader
        {
            public:
                IndexHeader()
                {
                    memset(this, 0, sizeof(IndexHeader));
                }

                BlockInfo block_info_;      // meta block info
                int32_t   bucket_size_;
                int32_t   data_file_offset_;
                int32_t   index_file_size_;
                int32_t   free_head_offset_; 
        };

        class IndexHandle
        {
            public:
                IndexHandle(const std::string& base_path, const int32_t main_block_id);
                
                ~IndexHandle();

                int create(const uint32_t logic_block_id, const int32_t buckect_size, const MMapOption mmap_option);

                int load(const uint32_t logic_block_id, const int32_t buckect_size, const MMapOption mmap_option);
        
                int remove(const uint32_t logic_block_id);

                int flush();

                int32_t get_data_offset() const
                {
                    return reinterpret_cast<IndexHeader*>(file_op_->get_mmap_data()) ->data_file_offset_;
                }

                void commit_block_data_offset(const int32_t file_size)
                {   
                    reinterpret_cast<IndexHeader*>(file_op_->get_mmap_data())->data_file_offset_ += file_size;
                }

                IndexHeader *index_header()
                {
                    return reinterpret_cast<IndexHeader*>(file_op_->get_mmap_data());
                }

                BlockInfo *block_info()
                {
                    return reinterpret_cast<BlockInfo*>(file_op_->get_mmap_data()) ;
                }

                int32_t bucket_size() const
                {
                    return reinterpret_cast<IndexHeader *>(file_op_->get_mmap_data())->bucket_size_;
                }

                int32_t* bucket_slot() const
                {
                    return reinterpret_cast<int32_t*>(reinterpret_cast<char*>(file_op_->get_mmap_data()) + sizeof(IndexHeader));
                }

                int32_t free_head_offset() const
                {
                    return reinterpret_cast<IndexHeader*>(file_op_->get_mmap_data())->free_head_offset_;
                }

                int update_block_info(const OperType oper_type, const uint32_t modify_size);

                int32_t write_segment_meta(const uint64_t key, MetaInfo& meta);

                int32_t read_segment_meta(const uint64_t key, MetaInfo& meta);

                int32_t delete_segment_meta(const uint64_t key);

                int32_t hash_find(const uint64_t key, int32_t& current_offset, int32_t& previous_offset) const;

                int32_t hash_insert(const uint64_t key, int32_t previous_offset, MetaInfo meta);

            private:
                bool hash_compare(const uint64_t key1, const uint64_t key2) const
                {
                    return (key1 == key2);
                }
    
            private:
                MMapFileOperation *file_op_;
                bool is_load_;
        };
    }
}












#endif // _QINIU_LARGEFILE_INDEX_HANDLE_H_
