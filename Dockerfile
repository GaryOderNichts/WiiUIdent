FROM ghcr.io/wiiu-env/devkitppc:20230417  

COPY --from=ghcr.io/wiiu-env/libmocha:20230417 /artifacts $DEVKITPRO

WORKDIR /project
