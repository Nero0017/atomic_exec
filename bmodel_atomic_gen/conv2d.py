from utils.gen_shell import generate
import torch
import torch.nn as nn

input_data = torch.tensor(range(16)).reshape(
    1, 1, 4, 4
).float() # (batch_size, input_channels, height)


print("Input shape:", input_data.shape)

conv_layer = nn.Conv2d(in_channels=1, out_channels=1, kernel_size=2, bias=False)

conv_layer.weight.data = torch.tensor(range(4)).float().reshape(conv_layer.weight.shape)

output_data = conv_layer(input_data)
print("Output shape:", output_data.shape)


generate("conv", conv_layer, [input_data], "conv_workspace")
