rm -rf conv_workspace
python3 conv2d.py
cp convert.sh conv_workspace/

pushd conv_workspace
bash convert.sh
popd
python3 gen_cmd.py

echo "ground truth"
python -c 'import numpy as np; print(np.load("./conv_workspace/conv_bm1684x_f32_model_outputs.npz")["2_Conv"].reshape(-1))'
