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
  fw_log("in mcu_cpy");
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
  fw_log("In cpy_l2s!!!");

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

// void tpu_kernel_pio(const void *args)
// {
//   // fw_log("In launch PIO !!!");
//   sg_api_launch_pio_t *param = (sg_api_launch_pio_t *)args;

//   static u64 *cur_cmd = NULL;
//   static u64 *cur_cmd_type = NULL;
//   if (!cur_cmd)
//   {
//     cur_cmd = (u64 *)param->bmodel_addr;
//     cur_cmd_type = (int8_t *)(param->bmodel_addr + param->allcmds_buf_size);
//   }
//   static int bdc_done = 0;
//   static int gdma_done = 0;

//   int forward_num = param->forward_num;
//   // CMD_ID_NODE *pid_node = (CMD_ID_NODE *)calloc(1, sizeof(CMD_ID_NODE));
//   // resync_cmd_id(pid_node);

//   fw_log("%llu, %llu", param->bmodel_addr, param->allcmds_buf_size);
//   fw_log("%d", forward_num);
//   fw_log("4 try!");
//   fw_log("%llu,%llu", cur_cmd, cur_cmd_type);
//   while (forward_num > 0)
//   {
//     fw_log("In while");
//     fw_log("ready to initialize");
//     // fw_log("%llu", cur_cmd_type[0]);
//     fw_log("%hhd", *((int8_t *)cur_cmd_type));

//     CMD_ID_NODE *pid_node = (CMD_ID_NODE *)calloc(1, sizeof(CMD_ID_NODE));
//     resync_cmd_id(pid_node);
//     pid_node->bd_cmd_id = 0;
//     pid_node->gdma_cmd_id = 0;
//     // if (cur_cmd_type[0] == 0)
//     // {
//     //   // fw_log("in launch bdc");
//     //   // pid_node->bd_cmd_id = (cur_cmd[0] >> 1) & 0xfffff;
//     //   // pid_node->gdma_cmd_id = (cur_cmd[0] >> 21) & 0xfffff;
//     //   pid_node->bd_cmd_id = 0;
//     //   pid_node->gdma_cmd_id = 0;

//     //   int bdc_bytes = get_bdc_cmd_len((u32 *)cur_cmd);
//     //   ASSERT(bdc_bytes % 8 == 0);
//     //   BEGIN_FAST_GEN_CMD(BD)
//     //   for (int i = 0; i < bdc_bytes / 16; i++)
//     //   {
//     //     u64 low = cur_cmd[0];
//     //     u64 high = cur_cmd[1];
//     //     WRITE_CMD_BD(i, high, low);
//     //     cur_cmd = cur_cmd + 2;
//     //   }
//     //   END_FAST_GEN_CMD(BD, pid_node)
//     //   cur_cmd_type++;
//     //   bdc_done++;
//     // }
//     fw_log("%d", cur_cmd_type[0]);
//     if (cur_cmd_type[0] == 1)
//     {
//       fw_log("in launch gdma");
//       // int gdma_id = (cur_cmd[0] >> 9) & 0xfffff;
//       // int gdma_dep_bdc_id = cur_cmd[1] & 0xfffff;
//       // pid_node->bd_cmd_id = gdma_dep_bdc_id;
//       // pid_node->gdma_cmd_id = gdma_id;
//       // pid_node->bd_cmd_id = 0;
//       // pid_node->gdma_cmd_id = 0;

//       int gdma_bytes = 96;
//       BEGIN_FAST_GEN_CMD(GDMA)
//       for (int i = 0; i < gdma_bytes / 16; i++)
//       {
//         u64 low = cur_cmd[0];
//         u64 high = cur_cmd[1];
//         WRITE_CMD_GDMA(i, high, low);
//         cur_cmd = cur_cmd + 2;
//       }
//       END_FAST_GEN_CMD(GDMA, pid_node)
//       cur_cmd_type++;
//       gdma_done++;
//     }
//     forward_num--;
//     fw_log("%d", forward_num);
//     tpu_poll();
//   }
//   // pid_node->bd_cmd_id = bdc_done - 1;
//   // pid_node->gdma_cmd_id = gdma_done - 1;
//   // poll_all_engine_done(pid_node);
//   // free(pid_node);
// }
// TPUKERNEL_FUNC_REGISTER(tpu_kernel_pio);

void tpu_kernel_launch_modified_pio(const void *args)
{
  static int calling_nums = 0;
  sg_api_pio_buf *param = (sg_api_pio_buf *)args;
  size_t tiu_buf_len = param->tiu_buf_len;
  int tiu_nums = param->tiu_nums;
  int dma_nums = param->dma_nums;
  u64 *bdc_cmd = (u64 *)(args + sizeof(sg_api_pio_buf));
  u64 *gdma_cmd = (u64 *)(args + sizeof(sg_api_pio_buf) + tiu_buf_len);
  int bdc_done = 0;
  int gdma_done = 0;

  CMD_ID_NODE *pid_node = (CMD_ID_NODE *)calloc(1, sizeof(CMD_ID_NODE));
  if (calling_nums == 0)
  {
    resync_cmd_id(pid_node);
    calling_nums++;
  }

  while (true)
  {
    int bd_id = 0, bd_dep_gdma_id = 0, gdma_id = 0, gdma_dep_bdc_id = 0;
    if (tiu_nums > 0)
    {
      bd_id = (bdc_cmd[0] >> 1) & 0xfffff;
      bd_dep_gdma_id = (bdc_cmd[0] >> 21) & 0xfffff;
    }

    if (dma_nums > 0)
    {
      gdma_id = (gdma_cmd[0] >> 9) & 0xfffff;
      gdma_dep_bdc_id = gdma_cmd[1] & 0xfffff;
    }

    if ((dma_nums == 0 && bdc_done + 1 <= tiu_nums) || (bd_id <= gdma_dep_bdc_id && bdc_done + 1 <= tiu_nums))
    {
      int bdc_bytes = get_bdc_cmd_len((u32 *)bdc_cmd);
      ASSERT(bdc_bytes % 8 == 0);
      BEGIN_FAST_GEN_CMD(BD)
      pid_node->bd_cmd_id = bd_id;
      pid_node->gdma_cmd_id = bd_dep_gdma_id;
      for (int i = 0; i < bdc_bytes / 16; i++)
      {
        u64 low = bdc_cmd[0];
        u64 high = bdc_cmd[1];
        WRITE_CMD_BD(i, high, low);
        bdc_cmd = bdc_cmd + 2;
      }
      END_FAST_GEN_CMD(BD, pid_node)
      bdc_done++;
    }
    else if (gdma_done + 1 <= dma_nums)
    {
      int gdma_bytes = 96;
      BEGIN_FAST_GEN_CMD(GDMA)
      pid_node->bd_cmd_id = gdma_dep_bdc_id;
      pid_node->gdma_cmd_id = gdma_id;
      for (int i = 0; i < gdma_bytes / 16; i++)
      {
        u64 low = gdma_cmd[0];
        u64 high = gdma_cmd[1];
        WRITE_CMD_GDMA(i, high, low);
        gdma_cmd = gdma_cmd + 2;
      }
      END_FAST_GEN_CMD(GDMA, pid_node)
      gdma_done++;
    }
    else
      break;
    poll_all_engine_done(pid_node);
  }
  free(pid_node);
}
TPUKERNEL_FUNC_REGISTER(tpu_kernel_launch_modified_pio);