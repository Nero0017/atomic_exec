pushd conv_workspace/conv_f32
gdb --args python $(which tdb.py)
popd

# b atomic_conv_binary32
# b /workspace/nntoolchain/TPU1686/bm1684x/cmodel/src/atomic_dma.cpp:1383
# p *(float*)tpu_global_mem_addr(src_start_offset)@16
