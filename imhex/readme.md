# ImHex Hex Editor support

[ImHex](https://github.com/WerWolv/ImHex) is an excellent hex editor for reverse engineering. It can display all details of the internal structure of a file. It supports a pattern language to easily support new file formats.

The flexemu project supports ImHex pattern files for the following file formats:

|Type                 |File extension             | Pattern file     |
|---------------------|---------------------------|------------------|
|FLEX binary files    |\*.cmd; \*.bin; \*.sys; ...|<code>flex_binary.hexpat</code>|
|FLEX disk image files|\*.dsk; \*.flx; \*.wta     |<code>flex_dskflx.hexpat</code>|
|FLEX random files    |unspecific                 |<code>flex_random.hexpat</code>|

These files are part of a flexemu installation. The path is operating system dependent.

| Operating System | Installation folder                                    |
|------------------|--------------------------------------------------------|
| UNIX like        |<code>&lt;prefix&gt;/share/flexemu/imhex/patterns</code>|
| Windows          |<code>&lt;InstallDirectory&gt;\ImHex\patterns</code>    |
