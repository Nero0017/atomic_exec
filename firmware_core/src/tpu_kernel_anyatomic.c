#include "bd_reg_def.h"
#include "firmware_common.h"
#include "nodechip_anyatomic.h"
#include "sg_api_struct.h"
#include <stdio.h>
#include <sys/time.h>
#include <assert.h>

static int call_pio_nums = 0;
// static int call_l2s_nums = 0;

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
  // call_l2s_nums++;
  // fw_log("In cpy_l2s!!!");
  // fw_log("call_l2s_nums: %d", call_l2s_nums);

  sg_api_mcu_cpy_t *param = (sg_api_mcu_cpy_t *)args;
  const dim4 shape = {1, 64, 512, 128};
  dim4 *shape_ptr = &shape;

  tpu_gdma_cpy_L2S(param->global_addr, 0, shape_ptr, NULL, NULL, DT_UINT32);
  tpu_poll();
}

TPUKERNEL_FUNC_REGISTER(tpu_kernel_gdma_cpy_l2s);

// bytes
static int get_bdc_cmd_len(
    u32 *cmd_buf)
{
  uint32_t len = 0;
  u32 tsk_type = (cmd_buf[1] >> 9) & 0xf;
  int eu_type = (cmd_buf[1] >> 13) & 0x1f;
  bool is_short = cmd_buf[0] & 0x1;
  if (tsk_type == 15)
  {
    len = 16;
  }
  else if (!is_short)
  {
    len = 128;
  }
  else if (tsk_type == 0 || tsk_type == 1)
  {
    len = 64;
  }
  else if (tsk_type == 6 || tsk_type == 13 || tsk_type == 14)
  {
    len = 48;
  }
  else if (tsk_type == 4 || tsk_type == 5 || tsk_type == 9 || tsk_type == 10)
  {
    len = 32;
  }
  else if (tsk_type == 2)
  {
    len = eu_type > 3 ? 32 : 48;
  }
  else if (tsk_type == 3)
  {
    len = (eu_type == 24 || eu_type == 25) ? 16 : 64;
  }
  else
  {
    ASSERT(0);
  }

  return len;
}

void tpu_kernel_pio(const void *args)
{
  call_pio_nums++;
  // fw_log("In tpu_kernel_pio!");
  // fw_log("call_pio_nums: %d", call_pio_nums);
  sg_api_pio_buf *param = (sg_api_pio_buf *)args;
  size_t tiu_buf_len = param->tiu_buf_len;
  int tiu_nums = param->tiu_nums;
  int dma_nums = param->dma_nums;
  u64 *bdc_cmd = (u64 *)(args + sizeof(sg_api_pio_buf));
  u64 *gdma_cmd = (u64 *)(args + sizeof(sg_api_pio_buf) + tiu_buf_len);
  int bdc_done = 0;
  int gdma_done = 0;

  tpu_initialize();
  CMD_ID_NODE id_node;
  CMD_ID_NODE *pid_node = &id_node;
  tpu_get_id_node(pid_node);
  // fw_log("nums tiu: %d, dma: %d", tiu_nums, dma_nums);
  // fw_log("starting bdc: %d", pid_node->bd_cmd_id);
  // fw_log("starting dma: %d", pid_node->gdma_cmd_id);
  // fw_log("Before while!");
  while (true)
  {
    int bd_id = (bdc_cmd[0] >> 1) & 0xfffff;
    int bd_dep_gdma_id = (bdc_cmd[0] >> 21) & 0xfffff;

    int gdma_id = (gdma_cmd[0] >> 9) & 0xfffff;
    int gdma_dep_bdc_id = gdma_cmd[1] & 0xfffff;

    // fw_log("bd_id: %d, bd_dep_gdma_id: %d, gdma_id: %d, gdma_dep_bdc_id: %d", bd_id, bd_dep_gdma_id, gdma_id, gdma_dep_bdc_id);

    if ((dma_nums == 0 && bdc_done + 1 <= tiu_nums) || (bd_id <= gdma_dep_bdc_id && bdc_done + 1 <= tiu_nums))
    {
      int bdc_bytes = get_bdc_cmd_len((u32 *)bdc_cmd);
      ASSERT(bdc_bytes % 8 == 0);
      bdc_cmd[0] &= ~((u64)0xfffff << 1);
      bdc_cmd[0] &= ~((u64)0xfffff << 21);
      assert(bdc_done + gdma_done < 1048576);
      bdc_cmd[0] |= ((bdc_done + 1) & 0xfffff) << 1;
      bdc_cmd[0] |= ((gdma_done) & 0xfffff) << 21;
      // fw_log("new bd id: %d", (bdc_cmd[0] >> 1) & 0xfffff);
      // fw_log("new bd dep id: %d", (bdc_cmd[0] >> 21) & 0xfffff);
      BEGIN_FAST_GEN_CMD(BD)
      pid_node->bd_cmd_id = bdc_done + 1;
      pid_node->gdma_cmd_id = gdma_done;
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
      gdma_cmd[0] &= ~((u64)0xfffff << 9);
      gdma_cmd[1] &= ~(0xfffff);
      assert(bdc_done + gdma_done < 1048576);
      gdma_cmd[0] |= ((gdma_done + 1) & 0xfffff) << 9;
      gdma_cmd[1] |= (bdc_done) & 0xfffff;
      // fw_log("new dma id: %d", (gdma_cmd[0] >> 9) & 0xfffff);
      // fw_log("new dma dep id: %d", gdma_cmd[1] & 0xfffff);
      BEGIN_FAST_GEN_CMD(GDMA)
      pid_node->bd_cmd_id = bdc_done;
      pid_node->gdma_cmd_id = gdma_done + 1;
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
    // fw_log("out if");
    // fw_log("out if bdc: %d", pid_node->bd_cmd_id);
    // fw_log("out if dma: %d", pid_node->gdma_cmd_id);
    poll_all_engine_done(pid_node);
  }
  // fw_log("out while");
  pid_node->bd_cmd_id = tiu_nums;
  pid_node->gdma_cmd_id = dma_nums;
  // fw_log("final bdc: %d", pid_node->bd_cmd_id);
  // fw_log("final dma: %d", pid_node->gdma_cmd_id);
  poll_all_engine_done(pid_node);
  // fw_log("END");
  tpu_poll();
  // fw_log("END2\n");
}
TPUKERNEL_FUNC_REGISTER(tpu_kernel_pio);

void tpu_kernel_pios(const void *args)
{
  call_pio_nums++;
  // fw_log("In tpu_kernel_pio!");
  // fw_log("call_pio_nums: %d", call_pio_nums);

  sg_api_pios_buf *param = (sg_api_pios_buf *)args;
  int tiu_nums = param->tiu_nums;
  int dma_nums = param->dma_nums;
  u64 *bdc_cmd = (u64 *)GET_GLOBAL_ADDR(param->tiu_addr);
  u64 *gdma_cmd = (u64 *)GET_GLOBAL_ADDR(param->dma_addr);
  int bdc_done = 0;
  int gdma_done = 0;

  tpu_initialize();
  CMD_ID_NODE id_node;
  CMD_ID_NODE *pid_node = &id_node;
  tpu_get_id_node(pid_node);
  // fw_log("nums tiu: %d, dma: %d", tiu_nums, dma_nums);
  // fw_log("starting bdc: %d", pid_node->bd_cmd_id);
  // fw_log("starting dma: %d", pid_node->gdma_cmd_id);
  // fw_log("Before while!");
  while (true)
  {
    int bd_id = (bdc_cmd[0] >> 1) & 0xfffff;
    int bd_dep_gdma_id = (bdc_cmd[0] >> 21) & 0xfffff;

    int gdma_id = (gdma_cmd[0] >> 9) & 0xfffff;
    int gdma_dep_bdc_id = gdma_cmd[1] & 0xfffff;

    // fw_log("bd_id: %d, bd_dep_gdma_id: %d, gdma_id: %d, gdma_dep_bdc_id: %d", bd_id, bd_dep_gdma_id, gdma_id, gdma_dep_bdc_id);

    if ((dma_nums == 0 && bdc_done + 1 <= tiu_nums) || (bd_id <= gdma_dep_bdc_id && bdc_done + 1 <= tiu_nums))
    {
      int bdc_bytes = get_bdc_cmd_len((u32 *)bdc_cmd);
      ASSERT(bdc_bytes % 8 == 0);
      bdc_cmd[0] &= ~((u64)0xfffff << 1);
      bdc_cmd[0] &= ~((u64)0xfffff << 21);
      assert(bdc_done + gdma_done < 1048576);
      bdc_cmd[0] |= ((bdc_done + 1) & 0xfffff) << 1;
      bdc_cmd[0] |= ((gdma_done) & 0xfffff) << 21;
      // fw_log("new bd id: %d", (bdc_cmd[0] >> 1) & 0xfffff);
      // fw_log("new bd dep id: %d", (bdc_cmd[0] >> 21) & 0xfffff);
      BEGIN_FAST_GEN_CMD(BD)
      pid_node->bd_cmd_id = bdc_done + 1;
      pid_node->gdma_cmd_id = gdma_done;
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
      gdma_cmd[0] &= ~((u64)0xfffff << 9);
      gdma_cmd[1] &= ~(0xfffff);
      assert(bdc_done + gdma_done < 1048576);
      gdma_cmd[0] |= ((gdma_done + 1) & 0xfffff) << 9;
      gdma_cmd[1] |= (bdc_done) & 0xfffff;
      // fw_log("new dma id: %d", (gdma_cmd[0] >> 9) & 0xfffff);
      // fw_log("new dma dep id: %d", gdma_cmd[1] & 0xfffff);
      BEGIN_FAST_GEN_CMD(GDMA)
      pid_node->bd_cmd_id = bdc_done;
      pid_node->gdma_cmd_id = gdma_done + 1;
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
    // fw_log("out if");
    // fw_log("out if bdc: %d", pid_node->bd_cmd_id);
    // fw_log("out if dma: %d", pid_node->gdma_cmd_id);
    poll_all_engine_done(pid_node);
  }
  // fw_log("out while");
  pid_node->bd_cmd_id = tiu_nums;
  pid_node->gdma_cmd_id = dma_nums;
  // fw_log("final bdc: %d", pid_node->bd_cmd_id);
  // fw_log("final dma: %d", pid_node->gdma_cmd_id);
  poll_all_engine_done(pid_node);
  // fw_log("END");
  tpu_poll();
  // fw_log("END2\n");
}
TPUKERNEL_FUNC_REGISTER(tpu_kernel_pios);
