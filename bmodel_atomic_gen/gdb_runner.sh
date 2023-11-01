pushd conv_workspace
gdb --args python $(which model_runner.py) --input conv_in_f32.npz --model conv_f32.bmodel --output conv_bm1684x_f32_model_outputs.npz
popd

# b atomic_conv_binary32
# b /workspace/nntoolchain/TPU1686/bm1684x/cmodel/src/atomic_dma.cpp:1383
# p *(float*)tpu_global_mem_addr(src_start_offset)@16
