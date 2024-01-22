#ifndef SG_API_STRUCT_H
#define SG_API_STRUCT_H

#ifndef WIN32
#define WITH_PLATFORM(x) __attribute__((packed)) x
#else
#define WITH_PLATFORM(x) x
#endif
// type define's
typedef unsigned long long u64;
typedef unsigned int u32;
typedef unsigned char u8;
typedef unsigned short u16;
#define NO_USE 0

#define MAX_ATTR_NUM 16
#define MAX_ID_ATTR_NUM 512
#define MAX_MULTI_CROP_NUM 10
#define MAX_SHAPE_DIMS 8

#define TPU_KERNEL_MAX_IMAGE_DIM 3
#define TPU_KERNEL_MAX_IMAGE_CHANNELS 3
#define MAX_bm_image_CHANNEL 4
#define MAX_PROPOSAL_LAYER_OUTPUT_ROI_NUM 1000

typedef struct {
  int length;
  int command_type;
  char data[128];
} WITH_PLATFORM(sg_api_atomic_t);

#define MAX_CMD_NUM 4

typedef struct {
  int cmd_num;
  int command_type[MAX_CMD_NUM];
  int command_length[MAX_CMD_NUM];
  char data[128 * MAX_CMD_NUM];
} WITH_PLATFORM(sg_api_atomics_t);

typedef struct {
  u64 input_global_offset;
  u64 output_global_offset;
  int shape[MAX_SHAPE_DIMS];
  int dims;
  float const_value;
  int inversed;
  int binary_type;
  int dtype;
  int if_relu;
  float relu_upper_limit;
} WITH_PLATFORM(tpu_kernel_api_const_binary_t);

#endif
