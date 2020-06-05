
## Notes

Other dependencies

-   cmake
-   libtiff-4
-   lrelease
-   sphinx-build-3
-   Boost

Do people have to use meson/ninja etc?

## Glossary

-   ISP: Image Signal Processor
-   SoC: System on a Chip(?)
-   Media controller API
-   UVC camera: USB Video Class
-   Applications need to know how to use this API and what devices are availabe
-   Application types supported by libcamera
    -   V4L2 - Video for Linux (compatibility)
    -   gstreamer: Multimedia framework
    -   libcamera native
    -   Android camera framework
    -   Language bindings for non-C++ applications
-   Different APIs
    -   Public API
    -   Internal APIs
        -   IPA: Image Processing Algorithm
        -   Pipeline handling
        -   IPC (Inter-process communications) based protocol