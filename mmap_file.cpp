#include "mmap_file.h"
#include <stdio.h>
#include <sys/mman.h>

static int debug = 1;

namespace qiniu
{
	namespace largefile
	{
		MMapFile::MMapFile():
		size_(0), fd_(-1), data_(NULL)
		{
		}
		
		MMapFile::MMapFile(const int fd):
		size_(0), fd_(fd), data_(NULL)
		{
		}
		
		MMapFile::MMapFile(const MMapOption& mmap_option, const int fd):
		size_(0), fd_(fd), data_(NULL)
		{
			mmap_file_option_.max_mmap_size_ = mmap_option.max_mmap_size_;
			mmap_file_option_.first_mmap_size_ = mmap_option.first_mmap_size_;
			mmap_file_option_.per_mmap_size_ = mmap_option.per_mmap_size_;
		}
		
		MMapFile::~MMapFile()
		{
			if (data_){
				if (debug) printf("mmap file destruct, fd:%d, mmapsize:%d, data:%p\n", fd_, size_, data_);
				msync(data_, size_, MS_SYNC);
				munmap(data_, size_);
				
				size_ = 0;
				data_ = NULL;
				
				mmap_file_option_.max_mmap_size_ = 0;
				mmap_file_option_.first_mmap_size_ = 0;
 				mmap_file_option_.per_mmap_size_ = 0;
			}
		}
		
		bool MMapFile::sync_file(){
			if (data_ != NULL && size_ > 0){
				return msync(data_, size_, MS_ASYNC) == 0;
			}
			return true;
		}
		
		bool MMapFile::map_file(const bool write){
			int flags = PROT_READ; // 定义写
			
			if (write){                // 判断读
				flags |= PROT_WRITE;
			}
			
			if (fd_ < 0) {   
				return false;
			}
			
			if (mmap_file_option_.max_mmap_size_ == 0){
				return false;
			}
			
			if (size_ < mmap_file_option_.max_mmap_size_) {  // 判断分配大小
				size_ = mmap_file_option_.first_mmap_size_;
			}else {
				size_ = mmap_file_option_.max_mmap_size_;
			}
			
			if (!ensure_file_size(size_)) {
				fprintf(stderr, "ensure file size failed in map_file, size:%d\n", size_);
				return false;
			}
			
			data_ = mmap(0, size_, flags, MAP_SHARED, fd_, 0);
			
			if (data_ == MAP_FAILED) {
				fprintf(stderr, "map file failed:%s", strerror(errno));
				
				size_ = 0;
				fd_ = -1;
				data_ = NULL;
				return false;
			}
			
			if (debug) printf("mmap file successed, fd:%d, mmapsize:%d, data:%p\n", fd_, size_, data_);
			
			return true;
		}
		
		void *MMapFile::get_data() const {
			return data_;
		}
		
		int32_t MMapFile::get_size() {
			return size_;
		}
		
		bool MMapFile::munmap_file() {
			if (munmap(data_, size_) == 0) {return true;}
			else {return false;}
		}
		
		bool MMapFile::remap_file() { // 扩容文件大小
			// 防御性编程
			if (fd_ < 0 || data_ == NULL) { // 首先判断文件fd和地址是否出错
				 fprintf(stderr, "mremap not mmaped yet\n");
				 return false;
			}
			
			if (size_ == mmap_file_option_.max_mmap_size_){ // 判断当前文件是否占用最大内存
				fprintf(stderr, "already mmaped max size, now size:%d, max size:%d\n", size_, mmap_file_option_.max_mmap_size_);
				return false;
			}
			
			int32_t new_size = size_ + mmap_file_option_.per_mmap_size_; // 新大小等于原有大小 + 每次固定属性分配大小
			if (new_size > mmap_file_option_.max_mmap_size_){
				new_size = mmap_file_option_.max_mmap_size_;
			}
			
			if (!ensure_file_size(new_size)){ // 其次根据新大小扩容
				fprintf(stderr, "ensure file size failed in map_file, size:%d\n", new_size);
				return false;
			}
			
			if (debug) printf("mremap start, fd:%d, now size:%d, old data:%p\n", fd_, new_size, data_);
			
			void *new_map_data = mremap(data_, size_, new_size, MREMAP_MAYMOVE);
			if (new_map_data == MAP_FAILED){
				fprintf(stderr, "mremap failed, new_size:%d, fd:%d, error desc:%s\n", new_size, fd_, strerror(errno));
				return false;
			}else {
				if (debug) printf("mremap successed, fd:%d, now_size:%d, old data:%p, new data:%p\n", fd_, new_size, data_, new_map_data);
			}
			
			data_ = new_map_data;
			size_ = new_size;
			return true;			
		}
		
		bool MMapFile::ensure_file_size(const int32_t size){
			struct stat s;  // 文件详细信息结构体获取
			if (fstat(fd_, &s) < 0){
				fprintf(stderr, "fstart error, error desc: %s\n", strerror(errno));
				return false;
			}
			
			if (s.st_size < size) {
				if (ftruncate(fd_, size) < 0 ) {
					fprintf(stderr, "ftruncate error, size:%d,, fd:%d, error desc:%s\n", size, fd_, strerror(errno));
					return false;
				}
			}
			
			return true;
		}
		
		
	}
}




























