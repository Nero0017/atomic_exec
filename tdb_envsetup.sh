
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

export PROJECT_ROOT=/workspace/tpu-mlir
export INSTALL_PATH=${INSTALL_PATH:-$PROJECT_ROOT/install}
export PYTHONPATH=$INSTALL_PATH/python:$PYTHONPATH
export PYTHONPATH=/usr/local/python_packages/:$PYTHONPATH
export PYTHONPATH=$PROJECT_ROOT/python:$PYTHONPATH
export PYTHONPATH=$PROJECT_ROOT/third_party/customlayer/python:$PYTHONPATH

export PATH=$INSTALL_PATH/bin:$PATH
export PATH=$PROJECT_ROOT/python/tools:$PATH
export PATH=$PROJECT_ROOT/python/utils:$PATH
export PATH=$PROJECT_ROOT/python/test:$PATH
export PATH=$PROJECT_ROOT/python/samples:$PATH
export PATH=$PROJECT_ROOT/third_party/customlayer/python:$PATH


export LIBSOPHON_TOP=/opt/sophon/libsophon-current
export CROSS_TOOLCHAINS=/workspace/bm_prebuilt_toolchains
# export LD_LIBRARY_PATH=/usr/local/libsophon-current/lib
export LD_LIBRARY_PATH=/usr/local/libsophon-current/lib:$DIR/build:$LD_LIBRARY_PATH
