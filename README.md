# libcula

`libcula` is a highly performant C library that provides an abstraction of countless interfaces in Linux. 

## Compiling

Install the dependencies:

- libuv
- libsystemd
- wayland
- cjson

And run these commands:

```bash
meson setup build
ninja -C build
```
