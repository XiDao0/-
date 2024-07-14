#include "index_handle.h"
#include "common.h"
#include "file_op.h"
#include <sstream>

using namespace qiniu;
using namespace largefile;

const static largefile::MMapOption mmap_option = {10240000, 4096, 4096};
const static uint32_t main_blocksize = 1024*1024*64;
const static uint32_t bucket_size = 1000;
static uint32_t block_id = 1;

static int debug = 1;

using namespace std;

int main(int argc, char **argv) {

    std::string mainblock_path;
    std::string index_path;
    int32_t ret = TFS_SUCCESS;

    cout << "Type your blockid " << endl;
    cin >> block_id;

    if (block_id < 0)
    {
        cerr << "invalid blockid, exit" << endl;
        exit(-1);
    }

//  加载索引块
    largefile::IndexHandle *index_handle = new largefile::IndexHandle(".", block_id);

    if (debug) printf("load index..\n");

    ret = index_handle->load(block_id, bucket_size, mmap_option);

    if (ret != largefile::TFS_SUCCESS)
    {
        fprintf(stderr, "load index_handle failed. reason: %s\n", strerror(errno));
        delete index_handle;
        exit(-2);
    }

    uint64_t file_no = 0;

    cout << "Type your file_id" << endl;
    cin >> file_no;
    
    if (file_no < 0)
    {
        cerr << "invalid file_id, exit" << endl;
        exit(-3);
    }

    // 删除文件 
    ret = index_handle->delete_segment_meta(file_no);
    if (ret != TFS_SUCCESS)
    {
        fprintf(stderr, "IndexHandle delete segment meta fail. ret: %d, file_no: %lu\n", ret, file_no);
        exit(-4);    
    }
    index_handle->flush();
    cout << "delete successfully !\n" << index_handle->free_head_offset() << endl;
    
    delete index_handle;

    return 0;
}
