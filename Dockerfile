FROM ghcr.io/wiiu-env/devkitppc:20230326 

COPY --from=ghcr.io/wiiu-env/libmocha:20220903 /artifacts $DEVKITPRO

# build and install latest wut
WORKDIR /
RUN \
mkdir wut && \
cd wut && \
git init . && \
git remote add origin https://github.com/devkitPro/wut.git && \
git fetch --depth 1 origin 451a1828f7646053b59ebacd813135e0300c67e8 && \
git checkout FETCH_HEAD
WORKDIR /wut
RUN make -j$(nproc)
RUN make install

# build and install latest sdl
WORKDIR /
RUN \
mkdir SDL && \
cd SDL && \
git init . && \
git remote add origin https://github.com/GaryOderNichts/SDL.git && \
git fetch --depth 1 origin 687746c8c9514b5d48d5f9665a1d5fa36c5e5547 && \
git checkout FETCH_HEAD
WORKDIR /SDL
RUN /opt/devkitpro/portlibs/wiiu/bin/powerpc-eabi-cmake -S. -Bbuild -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=$DEVKITPRO/portlibs/wiiu
RUN cmake --build build
RUN cmake --install build

WORKDIR /project
