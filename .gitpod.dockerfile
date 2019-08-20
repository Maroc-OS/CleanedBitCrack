FROM gitpod/workspace-full

### OpenCL ###
RUN sudo apt-get update \
    && apt-get install -yq \
        ocl-icd-opencl-dev \
        opencl-headers \
    && apt-get clean && rm -rf /var/lib/apt/lists/* /tmp/*