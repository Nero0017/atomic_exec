

model_transform.py \
   --model_name conv \
   --model_def conv.onnx \
   --input_shapes [[1,1,4,4]] \
   --test_input data.npz \
   --keep_aspect_ratio \
   --tolerance 0.85,0.7 \
   --debug \
   --test_result conv_top_output.npz \
   --mlir conv.mlir

model_deploy.py \
  --mlir conv.mlir \
  --quantize F32 \
  --chip bm1684x \
  --debug \
  --test_input conv_in_f32.npz \
  --test_reference conv_top_output.npz \
  --model conv_f32.bmodel

