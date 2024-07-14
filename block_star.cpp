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

    if (debug) printf("read index..\n");

    ret = index_handle->load(block_id, bucket_size, mmap_option);

    if (ret != largefile::TFS_SUCCESS)
    {
        fprintf(stderr, "read index_handle failed. reason: %s\n", strerror(errno));
        delete index_handle;
        exit(-2);
    }

    delete index_handle;

    return 0;
}
