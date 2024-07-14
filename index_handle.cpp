#include "index_handle.h"
#include "common.h"

static int debug = 1;

namespace qiniu {
    namespace largefile
    {
        IndexHandle::IndexHandle(const std::string& base_path, const int32_t main_block_id)
        {
            // create file_op_handle object
            std::stringstream tmp_stream;
            tmp_stream << base_path << INDEX_DIR_PREFIX << main_block_id;
            
            std::string index_path;
            tmp_stream >> index_path;

            file_op_ = new MMapFileOperation(index_path);
            is_load_ = false;
        }

        IndexHandle::~IndexHandle()
        {
            if (file_op_)
            {
                delete file_op_;
                file_op_ = NULL;
            }
        }

        int IndexHandle::create(const uint32_t logic_block_id, const int32_t bucket_size, const MMapOption mmap_option)
        {
            int ret = TFS_SUCCESS;

            if (debug)
            {
                printf("create index: block id: %u, buckect size: %d, max_mmap_size: %d, first_mmap_size: %d, per_mmap_size: %d\n", 
                        logic_block_id, bucket_size, mmap_option.max_mmap_size_, mmap_option.first_mmap_size_, mmap_option.per_mmap_size_);
            }

            if (is_load_)
            {
                return EXIT_INDEX_ALREADY_LOADED_ERROR;
            }

            int64_t file_size = file_op_->get_file_size();

            if (file_size < 0 )
            {
                return TFS_ERROR;
            }
            else if (file_size == 0)
            {
                IndexHeader i_header;
                i_header.block_info_.block_id_ = logic_block_id;
                i_header.block_info_.seq_no_ = 1;
                i_header.bucket_size_ = bucket_size;
                
                i_header.index_file_size_ = sizeof(IndexHeader) + bucket_size * sizeof(int32_t);

                char *init_data = new char[i_header.index_file_size_];

                memcpy(init_data, &i_header, sizeof(IndexHeader));
                memset(init_data + sizeof(IndexHeader), 0, (i_header.index_file_size_ - sizeof(IndexHeader)));

          //      std::cout << i_header.block_info_.seq_no_ << i_header.bucket_size_ << std::endl;

                ret = file_op_->pwrite_file(init_data, i_header.index_file_size_, 0);

//                std::cout << i_header.block_info_.seq_no_ << i_header.bucket_size_ << std::endl;

                delete[] init_data;
                init_data = NULL;

                if (ret != TFS_SUCCESS)
                {
                    return ret;
                }

//                ret = file_op_->flush_file();

  //              if (ret != TFS_SUCCESS)
      //          {
    //                return ret;
        //        }
            }
            else
            {
                return EXIT_META_UNEXPECT_FOUND_ERROR;
            }

            ret = file_op_->mmap_file(mmap_option);
            if (ret != TFS_SUCCESS)
            {
                return ret;
            }
        
            is_load_ = true;

//            std::cout << index_header()->index_file_size_ << block_info()->seq_no_ << std::endl;

            if (debug) printf("init block_id: %u index successful. data file size: %d, index file size: %d, buckect size: %d, free head offset: %d, seq_no: %u, filesize: %d, file_count: %d, del_size: %d, del_file_count: %d, version: %d\n", 
                    logic_block_id, index_header()->data_file_offset_, index_header()->index_file_size_, index_header()->bucket_size_,
                    index_header()->free_head_offset_, block_info()->seq_no_, block_info()->size_, block_info()->file_count_, block_info()->del_size_,
                    block_info()->del_file_count_, block_info()->version_);

            return TFS_SUCCESS;

        }

        
        int IndexHandle::load(const uint32_t logic_block_id, const int32_t bucketsize, const MMapOption mmap_option)
        {
            int ret = TFS_SUCCESS;

            if (is_load_)
            {
                return EXIT_INDEX_ALREADY_LOADED_ERROR;
            }

            int64_t file_size = file_op_->get_file_size();

            if (file_size < 0)
            {
                return file_size;
            }
            else if (file_size == 0)
            {
                return EXIT_INDEX_CORRUPT_ERROR;
            }
            else
            {
                MMapOption tmp_map_option = mmap_option;

                if (file_size > tmp_map_option.first_mmap_size_ && file_size < tmp_map_option.max_mmap_size_)
                {
                    tmp_map_option.first_mmap_size_ = file_size;
                }
        
                ret = file_op_->mmap_file(tmp_map_option);

                if (ret != TFS_SUCCESS)
                {
                    return ret;
                }

                if (0 == bucket_size() || 0 == block_info()->block_id_)
                {
                   fprintf(stderr, "Index corrupt error, block_id: %u, buctet_size: %d\n", block_info()->block_id_, bucket_size());
                   return EXIT_INDEX_CORRUPT_ERROR;
                }

                int32_t index_file_size = sizeof(IndexHeader) + bucketsize * sizeof(int32_t);

                if (file_size < index_file_size)
                {
                   fprintf(stderr, "Index corrupt error, block_id: %u, buctet_size: %d, file size: %ld\n", block_info()->block_id_, bucket_size(), file_size);
                   return EXIT_INDEX_CORRUPT_ERROR;
                }

                if (block_info()->block_id_ != logic_block_id)
                {
                    fprintf(stderr, "Index conflict, blockid: %u, index blockid: %u\n", logic_block_id, block_info()->block_id_);
                    return EXIT_BLOCKID_CONFLICT_ERROR;
                }

                if (bucketsize != bucket_size())
                {
                    fprintf(stderr, "Index configure, bucket_size: %d, index bucket_size: %d\n", bucketsize, bucket_size());
                    return EXIT_BUCKET_CONFIGURE_ERROR;
                }

                is_load_ = true;

                if (debug) printf("load block_id: %u index successful. data file size: %d, index file size: %d, buckect size: %d, free head offset: %d, seq_no: %u, filesize: %d, file_count: %d, del_size: %d, del_file_count: %d, version: %d\n", 
                        logic_block_id, index_header()->data_file_offset_, index_header()->index_file_size_, index_header()->bucket_size_,
                        index_header()->free_head_offset_, block_info()->seq_no_, block_info()->size_, block_info()->file_count_, block_info()->del_size_,
                        block_info()->del_file_count_, block_info()->version_);

                return TFS_SUCCESS;
            }
        }
        
        int IndexHandle::remove(const uint32_t logic_block_id)
        {
            if (is_load_)
            {
                if (logic_block_id != block_info()->block_id_)
                {
                    fprintf(stderr, "block id conflict: block id: %u, index block id: %u\n", logic_block_id, block_info()->block_id_);
                    return EXIT_BLOCKID_CONFLICT_ERROR;
                }
            }
    
            int ret = file_op_->munmap_file();
            if (ret != TFS_SUCCESS)
            {
                return ret;
            }

            ret = file_op_->unlink_file();
            return ret;
        }

        int IndexHandle::flush()
        {
            int ret = file_op_->flush_file();

            if (ret != TFS_SUCCESS)
            {
                fprintf(stderr, "index flush failed. ret: %d, reason: %s\n", ret, strerror(errno));
            }
            return ret;
        }

        int32_t IndexHandle::hash_find(const uint64_t key, int32_t& current_offset, int32_t& previous_offset) const
        {
            int ret = TFS_SUCCESS;
            MetaInfo meta_info;

            current_offset = 0;
            previous_offset = 0;
            
            int32_t slot = key % bucket_size();

            int32_t pos = bucket_slot()[slot];
            
            for (; pos != 0; )
            {
                ret = file_op_->pread_file(reinterpret_cast<char*>(&meta_info), sizeof(MetaInfo), pos);
                if (ret != TFS_SUCCESS)
                {
                    return ret;
                }

                if (hash_compare(key, meta_info.get_key()))
                {
                    current_offset = pos;
                    return TFS_SUCCESS;
                }
                
                previous_offset = pos;
                pos = meta_info.get_next_meta_offset();
            }

            return EXIT_META_NOT_FOUND_ERROR;
        }

        int32_t IndexHandle::hash_insert(const uint64_t key, const int32_t previous_offset, const MetaInfo meta)
        {
            int32_t slot = static_cast<uint32_t>(key) % bucket_size();
            MetaInfo tmp_meta_info;
            int32_t current_offset;
            int ret = TFS_SUCCESS;

            if (free_head_offset() != 0)
            {
                ret = file_op_->pread_file(reinterpret_cast<char*>(&tmp_meta_info), sizeof(MetaInfo), free_head_offset());
                if (ret != TFS_SUCCESS)
                {
                    return ret;
                }

                current_offset = free_head_offset();
                if (debug) printf("reuse metainfo, current_offset: %d\n", current_offset);
                index_header()->free_head_offset_ = tmp_meta_info.get_next_meta_offset();
            }else
            {
                current_offset = index_header()->index_file_size_;
                index_header()->index_file_size_ += sizeof(MetaInfo);
            }

            meta.set_next_meta_offset(0);

            MetaInfo tmp_meta;

            ret = file_op_->pwrite_file(reinterpret_cast<const char*>(&meta), sizeof(MetaInfo), current_offset);
            if (ret != TFS_SUCCESS)
            {
                index_header()->index_file_size_ -= sizeof(MetaInfo);
                return ret;   
            }

            if (0 != previous_offset)
            {
                ret = file_op_->pread_file(reinterpret_cast<char*>(&tmp_meta), sizeof(MetaInfo), previous_offset);
                if (ret != TFS_SUCCESS)
                {
                    return ret;
                }
                
                tmp_meta.set_next_meta_offset(current_offset);

                ret = file_op_->pwrite_file(reinterpret_cast<const char*>(&tmp_meta), sizeof(MetaInfo), previous_offset);
                if (ret != TFS_SUCCESS)
                {
                    return ret;
                }
            }else
            {
                bucket_slot()[slot] = current_offset;
            }
            return TFS_SUCCESS;

        }

        int32_t IndexHandle::write_segment_meta(const uint64_t key, MetaInfo& meta)
        {
            // 1 key是否存在
            int32_t current_offset = 0, previous_offset = 0;
            
            int ret = hash_find(key, current_offset, previous_offset);

            if (ret == TFS_SUCCESS)
            {
                return EXIT_META_UNEXPECT_FOUND_ERROR;
            }else if (ret != EXIT_META_NOT_FOUND_ERROR)
            {
                return ret;
            }           

            // 2 不存在即写入meta到哈希表中
            ret = hash_insert(key, previous_offset, meta);

            return ret;
        }

        int32_t IndexHandle::read_segment_meta(const uint64_t key, MetaInfo& meta)
        {
            int32_t current_offset = 0, previous_offset = 0;

            int32_t ret = hash_find(key, current_offset, previous_offset);
            
            if (ret == TFS_SUCCESS)
            {
                ret = file_op_->pread_file(reinterpret_cast<char*>(&meta), sizeof(MetaInfo), current_offset);
                return ret;
            }
            else 
            {
                return ret;
            }
        }

        int32_t IndexHandle::delete_segment_meta(const uint64_t key)
        {
            int32_t current_offset = 0, previous_offset = 0;

            int ret = hash_find(key, current_offset, previous_offset);
            if (ret != TFS_SUCCESS)
            {
                return ret;
            }

            MetaInfo meta;
            ret = file_op_->pread_file(reinterpret_cast<char*>(&meta), sizeof(MetaInfo), current_offset);
            if (ret != TFS_SUCCESS)
            {
                return ret;
            }

            int32_t pos = meta.get_next_meta_offset();

            if (previous_offset != 0)
            {
                MetaInfo tmp_meta;
                ret = file_op_->pread_file(reinterpret_cast<char*>(&tmp_meta), sizeof(MetaInfo), previous_offset);
                if (ret != TFS_SUCCESS)
                {
                    return ret;
                }

                tmp_meta.set_next_meta_offset(pos);
                ret = file_op_->pwrite_file(reinterpret_cast<const char*>(&tmp_meta), sizeof(MetaInfo), previous_offset);
                if (ret != TFS_SUCCESS)
                {   
                    return ret;
                }

            }else
            {
                int32_t slot = static_cast<uint32_t>(key) % bucket_size();
                bucket_slot()[slot] = pos;
            }
 
            // 把删除节点加入可重用内
            meta.set_next_meta_offset(free_head_offset());
            ret = file_op_->pwrite_file(reinterpret_cast<char*>(&meta), sizeof(MetaInfo), current_offset);
            if (ret !=TFS_SUCCESS)
            {
                return ret;
            }

            index_header()->free_head_offset_ = current_offset;


            update_block_info(C_OPER_DELETE, meta.get_file_size());
            if (debug) printf("delete segment metainfo. reuse metainfo, free_head_offser: %d, current_offset: %d\n", current_offset, free_head_offset());

            return TFS_SUCCESS;
        }

        int IndexHandle::update_block_info(const OperType oper_type, const uint32_t modify_size)
        {
            if (block_info()->block_id_ == 0)
            {
                return EXIT_BLOCKID_ZERO_ERROR;
            }
            // 插入
            if (oper_type == C_OPER_INSERT)
            {
                ++block_info()->version_;
                ++block_info()->file_count_;
                ++block_info()->seq_no_;
                block_info()->size_ += modify_size;
            }else if (oper_type == C_OPER_DELETE) // 删除
            {
                ++block_info()->version_;
                --block_info()->file_count_;
                --block_info()->del_file_count_;
                block_info()->del_size_ += modify_size;
                block_info()->size_ -= modify_size;
            }

            if (debug) printf("update block info: block_id: %u, seq_no: %u, size: %d, file_count: %d, del_size: %d, del_file_count: %d, version: %d, OperType: %d\n", 
                    block_info()->block_id_, block_info()->seq_no_, block_info()->size_, block_info()->file_count_, block_info()->del_size_,
                    block_info()->del_file_count_, block_info()->version_, oper_type);

            return TFS_SUCCESS;
        }
    }
}
