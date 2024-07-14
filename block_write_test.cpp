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
//bucket_size
    ret = index_handle->load(block_id, bucket_size, mmap_option);

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
    

    largefile::FileOperation *mainblock = new largefile::FileOperation(mainblock_path, O_RDWR);
        
    char buffer[4096];

    memset(buffer, '6', sizeof(buffer));

    int32_t data_offset = index_handle->get_data_offset();
    uint32_t file_no = index_handle->block_info()->seq_no_;

    if (ret = mainblock->pwrite_file(buffer, sizeof(buffer), data_offset) != TFS_SUCCESS)
    {
        fprintf(stderr, "mainblock write failed. ret: %d, reason: %s\n", ret, strerror(errno));

        mainblock->close_file();
        delete index_handle;
        delete mainblock;
        exit(-3);
    }

    MetaInfo meta;
    meta.set_file_id(file_no);
    meta.set_inner_offset(data_offset);
    meta.set_file_size(sizeof(buffer));

    ret = index_handle->write_segment_meta(meta.get_key(), meta);
    if (ret == TFS_SUCCESS)
    {
        index_handle->commit_block_data_offset(sizeof(buffer));

        index_handle->update_block_info(C_OPER_INSERT, sizeof(buffer));

        ret = index_handle->flush();
        if (ret != TFS_SUCCESS)
        {
            fprintf(stderr, "update main_block: %u info failed. file_no: %u\n", block_id, file_no);
        }
    }else
    {
        fprintf(stderr, "write_segment_meta - mainblock %u failed. file no: %u\n", block_id, file_no);
    }

    if (ret != TFS_SUCCESS)
    {
        fprintf(stderr, "write into main_block: %u info failed. file_no: %u\n", block_id, file_no);
    }else
    {
        if (debug) printf("write successfully. file no: %u, block_id: %u\n", file_no, block_id);
    }

    mainblock->close_file();


    delete index_handle;
    delete mainblock;


    return 0;
}
