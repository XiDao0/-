#ifndef WYX_LARGEFILE_MMAPFILE_H_
#define WYX_LARGEFILE_MMAPFILE_H_

#include <unistd.h> // 常规函数
#include "common.h"

namespace qiniu
{
	namespace largefile
	{
		
		
		class MMapFile
		{
		  public:
			MMapFile(); // 构造
			
			explicit MMapFile(const int fd); // 带参构造
			MMapFile(const MMapOption& mmap_option, const int fd); // 根据文件ID设置
			
			~MMapFile();
			
			bool sync_file(); 					  	 // 文件同步调用
			bool map_file(const bool write = false); // 实际文件分配到内存函数
			void *get_data() const; // 获取内存分配首地址
			int32_t get_size();     // 获取分配内存大小
			
			bool munmap_file(); // 释放内存
			bool remap_file(); 	 // 重新执行映射
			
		  private:
			bool ensure_file_size(const int32_t size); // 扩容
			
		  private:
		  int32_t size_;
		  int fd_;
		  void *data_;
		  
		  struct MMapOption mmap_file_option_;
			
			
		};
	}
}





#endif // WYX_LARGEFILE_MMAPFILE_H_
