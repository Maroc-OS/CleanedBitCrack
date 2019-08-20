FROM gitpod/workspace-full

### C/C++ ###
RUN curl -fsSL https://apt.llvm.org/llvm-snapshot.gpg.key | apt-key add - \
    && apt-add-repository -yu "deb http://apt.llvm.org/disco/ llvm-toolchain-disco main" \
    && sudo apt-get update \
    && apt-get install -yq \
        libllvm7 \
        llvm-7 \
        llvm-7-runtime \
        clang-9 \
        clang-format-9 \
        clang-tidy-9 \
        clang-tools-9 \
        cmake \
        gdb \
        lld-9 \
        ocl-icd-opencl-dev \
        opencl-headers \
    && ln -s /usr/bin/clangd-9 /usr/bin/clangd \
    && apt-get clean && rm -rf /var/lib/apt/lists/* /tmp/*