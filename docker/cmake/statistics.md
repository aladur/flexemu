## Docker build statistics

Build time and docker image size for different linux distributions.
The build command is:

```
   docker build --rm --no-cache --pull -f <distribution>/Dockerfile -t <image-name>:<tag> .
```

|build time<br>[s]|docker image|image size<br>[GByte]|Qt version|user name|remark|
|-|-|-|-|-|-|
351|almalinux|1.93|6.10.1|user|[3]|
338|alpine|1.95|5.15.18|user||
905|altlinux|3.36|5.15.18|user||
335|archlinux|3.08|5.15.19|user||
266|biglinux|2.51|5.15.14|builder|[4]|
321|cachyos|3.36|5.15.19|user||
274|centos|2.13|5.15.9|user||
1094|debian13|1.94|5.15.15|user||
1373|debian13_be|1.52|5.15.8|user|[1][5]|
1057|debian13_i386|2.00|5.15.8|user|[2][6]|
1146|debian13_qt6|1.95|6.8.2|user||
1631|debian_testing|2.17|5.15.19|user||
1217|debian_unstable|2.17|5.15.19|user||
1180|debian_unstable_qt6|2.20|6.10.2|user||
2225|endlessos|2.86|5.15.8|user||
428|fedora|2.21|5.15.18|user||
1121|kalilinux|2.51|5.15.19|user||
325|linuxmint|3.56|5.15.13|ubuntu||
809|mageia|2.99|5.15.7|user||
761|manjaro|4.36|5.15.19|builder||
936|opensuse|2.19|5.15.17|user||
624|oraclelinux|2.33|6.9.1|user|[3]|
428|rockylinux|1.84|6.10.1|user|[3]|
509|slackware|3.29|5.15.3|user||
1477|ubuntu|2.09|5.15.18|ubunutu||
917|voidlinux|3.63|5.15.11|user||

[1] use additional parameter `--platform=linux/s390x`<br>
[2] use additional parameter `--platform=linux/386`<br>
[3] only Qt6 supported<br>
[4] Fails: 20260709 reproducible PROTOCOL_ERROR issues<br>
[5] Fails: 20260709 reproducible segmentation fault compiling joystick.cpp<br>
[6] Fails: 20260709 image is generated but not usable<br>

## Linux distribution dependency tree

```
alpine
altlinux
archlinux -+-> manjaro ---> biglinux
           +-> cachyos
debian -+-> ubuntu ---> linuxmint
        +-> kalilinux
        +-> endlessos
opensuse
(redhat) -+-> fedora -+-> almalinux
          |           +-> centos
          +-> oraclelinux
          +-> rockylinux
          +-> (mandrake) ---> (mandriva) ---> mageia
slackware
voidlinux
```
