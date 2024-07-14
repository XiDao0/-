#include "index_handle.h"
#include "common.h"
#include "file_op.h"
#include <sstream>

using namespace qiniu;
using namespace largefile;

const static largefile::MMapOption mmap_option = {10240000, 4096, 4096};
const static uint32_t main_blocksize = 1024*1024*64;
const static uint32_t bucket_size = 1000;
static int32_t block_id = 1;

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

//  创建索引块
    largefile::IndexHandle *index_handle = new largefile::IndexHandle(".", block_id);

    if (debug) printf("init index..\n");
//bucket_size
    ret = index_handle->create(block_id, bucket_size, mmap_option);

    if (ret != largefile::TFS_SUCCESS)
    {
        fprintf(stderr, "create index_handle failed. reason: %s\n", strerror(errno));
        delete index_handle;
        exit(-2);
    }

// 创建主块

    std::stringstream tmp_stream;
    tmp_stream << "." << MAINBLOCK_DIR_PREFIX << block_id;
    tmp_stream >> mainblock_path;
    

    largefile::FileOperation *mainblock = new largefile::FileOperation(mainblock_path, O_RDWR | O_LARGEFILE | O_CREAT);

    ret = mainblock->ftruncate_file(main_blocksize);

    if (ret != 0)
    {
        fprintf(stderr, "create main block: %s failed. reason: %s\n", mainblock_path.c_str(), strerror(errno));
        index_handle->remove(block_id);
        delete mainblock;
        
        exit(-3);
    }

 //   std::stringstream tmp_index_stream;
   // tmp_index_stream << "." << INDEX_DIR_PREFIX << block_id;
    //tmp_index_stream >> index_path;


    cout << "index_handle、mainblock  success" << endl;
    
    mainblock->close_file();
    index_handle->flush();

    delete index_handle;
    delete mainblock;


    return 0;
}
