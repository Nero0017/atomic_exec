#include "bd_reg_def.h"
#include "firmware_common.h"
#include "nodechip_anyatomic.h"

void tpu_kernel_launch_atomic_gdma(const void *args) {

  tpu_initialize();

  CMD_ID_NODE id_node;
  id_node.bd_cmd_id = 0;
  id_node.gdma_cmd_id = 1;

  CMD_ID_NODE *pid_node = &id_node;
  u64 high, low;
  // 1
  {
    BEGIN_FAST_GEN_CMD(GDMA)
    low = 2199023256102;
    high = 0;
    WRITE_CMD_GDMA(0, high, low);
    low = 17179869188;
    high = 4294967297;
    WRITE_CMD_GDMA(1, high, low);
    low = 17179869188;
    high = 4294967297;
    WRITE_CMD_GDMA(2, high, low);
    low = 281492156645377;
    high = 0;
    WRITE_CMD_GDMA(3, high, low);
    low = 4294967296;
    high = 134217728;
    WRITE_CMD_GDMA(4, high, low);
    low = 0;
    high = 18446744073709551615;
    WRITE_CMD_GDMA(5, high, low);

    END_FAST_GEN_CMD(GDMA, pid_node)
  }
  // 2
  {
    BEGIN_FAST_GEN_CMD(GDMA)
    low = 2199023256614;
    high = 0;
    WRITE_CMD_GDMA(0, high, low);
    low = 68719476752;
    high = 4294967300;
    WRITE_CMD_GDMA(1, high, low);
    low = 68719476752;
    high = 4294967300;
    WRITE_CMD_GDMA(2, high, low);
    low = 1125917086777345;
    high = 0;
    WRITE_CMD_GDMA(3, high, low);
    low = 4294971392;
    high = 134234112;
    WRITE_CMD_GDMA(4, high, low);
    low = 0;
    high = 18446744073709551615;
    WRITE_CMD_GDMA(5, high, low);
    END_FAST_GEN_CMD(GDMA, pid_node)
  }
  // 3
  {
    BEGIN_FAST_GEN_CMD(BD)
    low = 36028797023158275;
    high = 6755408601415972;
    WRITE_CMD_BD(0, high, low);
    low = 281479271677952;
    high = 1125904202006531;
    WRITE_CMD_BD(1, high, low);
    low = 8590065668;
    high = 16384;
    WRITE_CMD_BD(2, high, low);
    low = 0;
    high = 0;
    WRITE_CMD_BD(3, high, low);
    END_FAST_GEN_CMD(BD, pid_node)
  }
  // 4
  {
    BEGIN_FAST_GEN_CMD(GDMA)
    low = 2199023257126;
    high = 1;
    WRITE_CMD_GDMA(0, high, low);
    low = 68719476752;
    high = 4294967299;
    WRITE_CMD_GDMA(1, high, low);
    low = 38654705673;
    high = 4294967299;
    WRITE_CMD_GDMA(2, high, low);
    low = 844437815099393;
    high = 0;
    WRITE_CMD_GDMA(3, high, low);
    low = 134266880;
    high = 4294975488;
    WRITE_CMD_GDMA(4, high, low);
    low = 0;
    high = 18446744073709551615;
    WRITE_CMD_GDMA(5, high, low);
    END_FAST_GEN_CMD(GDMA, pid_node)
  }
  tpu_poll();
}
TPUKERNEL_FUNC_REGISTER(tpu_kernel_launch_atomic_gdma);
