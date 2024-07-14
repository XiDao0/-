#include <iostream>
#include "mmap_file.h"
#include "common.h"

using namespace std;
using namespace qiniu;

const static mode_t OPEN_MODE = 0644;
const static largefile::MMapOption mmap_option = {10240000, 4096, 4096}; // 文件映射大小
 
int open_file(string file_name, int open_flags){
	int fd = open(file_name.c_str(), open_flags, OPEN_MODE); // 成功返回>0
	if (fd < 0){
		return -errno; // errno strerror(errno) 
	}
	
	return fd;
}

int main(void) {
	
	const char *filename = "./mapfile_test.txt";
	
	// 打开/ 创建一个文件  open函数
	int fd = open_file(filename, O_RDWR | O_CREAT | O_LARGEFILE);

	if (fd < 0) {
		fprintf(stderr, "open file failed, filename:%s, error desc:%s\n", filename, strerror(-fd));
	}
	
	largefile::MMapFile *map_file = new largefile::MMapFile(mmap_option, fd);
	
	bool is_mmaped = map_file->map_file(true);
	
	if (is_mmaped) {
		
		map_file->remap_file(); //扩容
		
		memset(map_file->get_data(), '8', map_file->get_size());
		map_file->sync_file();
		map_file->munmap_file();
	}
	
	close(fd);
	
	
	return 0;
}
