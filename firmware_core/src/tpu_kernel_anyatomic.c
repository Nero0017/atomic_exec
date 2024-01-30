#include "bd_reg_def.h"
#include "firmware_common.h"
#include "nodechip_anyatomic.h"
#include "sg_api_struct.h"
#include <stdio.h>
#include <sys/time.h>

void tpu_kernel_launch_atomic(const void *args)
{
  sg_api_atomic_t *param = (sg_api_atomic_t *)args;
  tpu_initialize();

  CMD_ID_NODE id_node;
  id_node.bd_cmd_id = 0;
  id_node.gdma_cmd_id = 0;

  CMD_ID_NODE *pid_node = &id_node;
  u64 high, low;
  u64 *p_cmd = (u64 *)param->data;
  if (param->command_type == 0)
  {
    BEGIN_FAST_GEN_CMD(BD)
    for (int i = 0; i < param->length / 16; i++)
    {
      low = *p_cmd;
      p_cmd++;
      high = *p_cmd;
      p_cmd++;
      WRITE_CMD_BD(i, high, low);
      printf("WRITE_CMD_BD(%d, %llu, %llu)\n", i, high, low);
    }
    END_FAST_GEN_CMD(BD, pid_node)
  }
  else
  {
    BEGIN_FAST_GEN_CMD(GDMA)
    for (int i = 0; i < param->length / 16; i++)
    {
      low = *p_cmd;
      p_cmd++;
      high = *p_cmd;
      p_cmd++;
      WRITE_CMD_GDMA(i, high, low);
      printf("WRITE_CMD_GDMA(%d, %llu, %llu)\n", i, high, low);
    }
    END_FAST_GEN_CMD(GDMA, pid_node)
  }

  tpu_poll();
}

TPUKERNEL_FUNC_REGISTER(tpu_kernel_launch_atomic);

void tpu_kernel_mcu_cpy_l2s(const void *args)
{
  sg_api_mcu_cpy_t *param = (sg_api_mcu_cpy_t *)args;

  const unsigned char *local_data =
      (const unsigned char *)tpu_local_mem_addr(0, 0);
  const unsigned char *global_data =
      (const unsigned char *)tpu_global_mem_addr(param->global_addr);
  memcpy(global_data, local_data, 16 * 1024 * 1024);
  tpu_flush_cache(param->global_addr, 16 * 1024 * 1024);
}

// void tpu_kernel_mcu_cpy_l2s(const void *args)
// {
//   // struct timeval start, mid, end;
//   // gettimeofday(&start, NULL);
//   // fw_log("mcu cpy start: %d.%d", start.tv_sec, start.tv_usec);

//   sg_api_mcu_cpy_t *param = (sg_api_mcu_cpy_t *)args;

//   const unsigned char *local_data =
//       (const unsigned char *)tpu_local_mem_addr(0, 0);
//   const unsigned char *global_data =
//       (const unsigned char *)tpu_global_mem_addr(param->global_addr);
//   memcpy(global_data, local_data, 16 * 1024 * 1024);
//   // gettimeofday(&mid, NULL);
//   tpu_flush_cache(param->global_addr, 16 * 1024 * 1024);
//   // gettimeofday(&end, NULL);
//   // fw_log("mcu cpy end: %d.%d", end.tv_sec, end.tv_usec);
//   // fw_log("mcu cpy cost: %d.%d", end.tv_sec, end.tv_usec);
//   // unsigned int *time_data =
//   //     (const unsigned char *)tpu_global_mem_addr(param->global_addr + 16 * 1024 * 1024);
//   // time_data[0] = start.tv_sec;
//   // time_data[1] = start.tv_usec;
//   // time_data[2] = mid.tv_sec;
//   // time_data[3] = mid.tv_usec;
//   // time_data[4] = end.tv_sec;
//   // time_data[5] = end.tv_usec;
//   // tpu_flush_cache(param->global_addr + 16 * 1024 * 1024, 4 * 6);
// }
TPUKERNEL_FUNC_REGISTER(tpu_kernel_mcu_cpy_l2s);

void tpu_kernel_gdma_cpy_l2s(const void *args)
{
  tpu_initialize();

  sg_api_mcu_cpy_t *param = (sg_api_mcu_cpy_t *)args;
  const dim4 shape = {1, 64, 512, 128};
  dim4 *shape_ptr = &shape;

  tpu_gdma_cpy_L2S(param->global_addr, 0, shape_ptr, NULL, NULL, DT_UINT32);
  tpu_poll();
}

TPUKERNEL_FUNC_REGISTER(tpu_kernel_gdma_cpy_l2s);

void tpu_kernel_launch_atomics(const void *args)
{
  sg_api_atomics_t *param = (sg_api_atomics_t *)args;
  tpu_initialize();

  CMD_ID_NODE id_node;
  id_node.bd_cmd_id = 0;
  id_node.gdma_cmd_id = 0;

  CMD_ID_NODE *pid_node = &id_node;
  size_t offset = 0;
  for (int i = 0; i < param->cmd_num; i++)
  {
    size_t length = param->command_length[i];
    int command_type = param->command_type[i];

    u64 high, low;
    u64 *p_cmd = (u64 *)(param->data + offset);
    if (command_type == 0)
    {
      BEGIN_FAST_GEN_CMD(BD)
      for (int i = 0; i < length / 16; i++)
      {
        low = *p_cmd;
        p_cmd++;
        high = *p_cmd;
        p_cmd++;
        printf("WRITE_CMD_BD(%d, %llu, %llu)", i, high, low);
        WRITE_CMD_BD(i, high, low);
      }
      END_FAST_GEN_CMD(BD, pid_node)
    }
    else
    {
      BEGIN_FAST_GEN_CMD(GDMA)
      for (int i = 0; i < length / 16; i++)
      {
        low = *p_cmd;
        p_cmd++;
        high = *p_cmd;
        p_cmd++;
        printf("WRITE_CMD_GDMA(%d, %llu, %llu)", i, high, low);
        WRITE_CMD_GDMA(i, high, low);
      }
      END_FAST_GEN_CMD(GDMA, pid_node)
    }
    offset += length;
  }
  tpu_poll();
}
TPUKERNEL_FUNC_REGISTER(tpu_kernel_launch_atomics);

void tpu_kernel_launch_atomic_example(const void *args)
{
  tpu_initialize();

  CMD_ID_NODE id_node;
  id_node.bd_cmd_id = 0;
  id_node.gdma_cmd_id = 1;

  CMD_ID_NODE *pid_node = &id_node;
  u64 high, low;
  {
    u64 high = 0, low = 0;
    BEGIN_FAST_GEN_CMD(BD)
    low =
        ((u64)pid_node->gdma_cmd_id << 17) | ((u64)15 << 41) | ((u64)31 << 45);
    WRITE_CMD_BD(0, high, low);
    END_FAST_GEN_CMD(BD, pid_node)
  }

  {
    u64 low = 0;
    BEGIN_FAST_GEN_CMD(GDMA)
    low = (1ull << 3) | (1ull << 5) | ((u64)6 << 32) | (u64)0 << 36;
    WRITE_CMD_GDMA(0, 0ull, low);
    END_FAST_GEN_CMD(GDMA, pid_node)
  }
  // 1
  {
    BEGIN_FAST_GEN_CMD(GDMA)
    low = 2199023256102UL;
    high = 0UL;
    WRITE_CMD_GDMA(0, high, low);
    low = 17179869188UL;
    high = 4294967297UL;
    WRITE_CMD_GDMA(1, high, low);
    low = 17179869188UL;
    high = 4294967297UL;
    WRITE_CMD_GDMA(2, high, low);
    low = 281492156645377UL;
    high = 0UL;
    WRITE_CMD_GDMA(3, high, low);
    low = 4294967296UL;
    high = 134217728UL;
    WRITE_CMD_GDMA(4, high, low);
    low = 0UL;
    high = 18446744073709551615UL;
    WRITE_CMD_GDMA(5, high, low);

    END_FAST_GEN_CMD(GDMA, pid_node)
  }
  // 2
  {
    BEGIN_FAST_GEN_CMD(GDMA)
    low = 2199023256614UL;
    high = 0UL;
    WRITE_CMD_GDMA(0, high, low);
    low = 68719476752UL;
    high = 4294967300UL;
    WRITE_CMD_GDMA(1, high, low);
    low = 68719476752UL;
    high = 4294967300UL;
    WRITE_CMD_GDMA(2, high, low);
    low = 1125917086777345UL;
    high = 0UL;
    WRITE_CMD_GDMA(3, high, low);
    low = 4294971392UL;
    high = 134234112UL;
    WRITE_CMD_GDMA(4, high, low);
    low = 0UL;
    high = 18446744073709551615UL;
    WRITE_CMD_GDMA(5, high, low);
    END_FAST_GEN_CMD(GDMA, pid_node)
  }
  // 3
  {
    BEGIN_FAST_GEN_CMD(BD)
    low = 36028797023158275UL;
    high = 6755408601415972UL;
    WRITE_CMD_BD(0, high, low);
    low = 281479271677952UL;
    high = 1125904202006531UL;
    WRITE_CMD_BD(1, high, low);
    low = 8590065668UL;
    high = 16384UL;
    WRITE_CMD_BD(2, high, low);
    low = 0UL;
    high = 0UL;
    WRITE_CMD_BD(3, high, low);
    END_FAST_GEN_CMD(BD, pid_node)
  }
  // 4
  {
    BEGIN_FAST_GEN_CMD(GDMA)
    low = 2199023257126UL;
    high = 1UL;
    WRITE_CMD_GDMA(0, high, low);
    low = 68719476752UL;
    high = 4294967299UL;
    WRITE_CMD_GDMA(1, high, low);
    low = 38654705673UL;
    high = 4294967299UL;
    WRITE_CMD_GDMA(2, high, low);
    low = 844437815099393UL;
    high = 0UL;
    WRITE_CMD_GDMA(3, high, low);
    low = 134266880UL;
    high = 4294975488UL;
    WRITE_CMD_GDMA(4, high, low);
    low = 0UL;
    high = 18446744073709551615UL;
    WRITE_CMD_GDMA(5, high, low);
    END_FAST_GEN_CMD(GDMA, pid_node)
  }
  tpu_poll();
}
TPUKERNEL_FUNC_REGISTER(tpu_kernel_launch_atomic_example);
