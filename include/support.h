#ifndef __SUPPORT_H__
#define __SUPPORT_H__

#define ASSERT_SUCCESS(command)                                                                               \
    {                                                                                                         \
        bm_status_t status_;                                                                                  \
        status_ = command;                                                                                    \
        if (status_ != BM_SUCCESS)                                                                            \
        {                                                                                                     \
            std::cerr << #command << "  " << __func__ << ":" << __LINE__ << "   command failed" << std::endl; \
            exit(-1);                                                                                         \
        }                                                                                                     \
    }

void print_data(char *data, size_t size)
{
    for (int i = 0; i < size; i++)
    {
        std::cout << data[i];
    }
    std::cout << std::endl;
}

#endif