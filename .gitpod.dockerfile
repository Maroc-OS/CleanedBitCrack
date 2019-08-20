FROM gitpod/workspace-full

RUN sudo apt-get update \
 && sudo apt-get install -y \
    clang-7 ocl-icd-opencl-dev opencl-headers \
 && sudo rm -rf /var/lib/apt/lists/*
