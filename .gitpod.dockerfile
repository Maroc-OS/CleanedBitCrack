FROM gitpod/workspace-full

### OpenCL ###
RUN sudo apt-get update \
    && sudo apt-get install -yq \
        ocl-icd-opencl-dev \
        ocl-icd-libopencl1 \
        opencl-headers \
    && sudo apt-get clean && sudo rm -rf /var/lib/apt/lists/* /tmp/*
