## Docker build statistics

Build time and docker image size for different linux distributions.
The build command is:

```
   docker build --rm --no-cache -t <image-name>:<tag> .
```

|build time<br>[s]|docker image|image size<br>[GByte]|Qt version|user name|remark|
|-|-|-|-|-|-|
248|almalinux|1.57|5.15.9|user||
285|alpine|1.48|5.15.10|user||
751|altlinux|2.11|5.15.13|user||
323|archlinux|3.12|5.15.14|user||
266|biglinux|2.51|5.15.14|builder||
274|centos|1.65|5.15.9|user||
985|debian|1.60|5.15.8|user||
1552|debian_be|1.52|5.15.8|user|1)|
948|debian_i368|1.48|5.15.8|user|2)|
959|debian_qt6|1.59|6.4.2|user||
1053|debian_testing|1.65|5.15.13|user||
1069|debian_testing_qt6|1.66|6.6.2|user||
1341|debian_unstable|1.68|5.15.13|user||
1199|debian_unstable_qt6|1.69|6.6.2|user||
1861|endlessos|2.19|5.15.8|user||
336|fedora|1.95|5.15.14|user||
1121|kalilinux|1.78|5.15.10|user||
360|linuxmint|2.72|5.15.13|ubuntu||
943|mageia|2.13|5.15.7|user||
342|manjaro|2.70|5.15.14|builder||
996|opensuse|1.99|5.15.12|user||
398|oraclelinux|1.86|5.15.9|user||
303|rockylinux|1.58|5.15.9|user||
582|slackware|2.45|5.15.3|user||
1101|ubuntu|1.68|5.15.13|ubunutu||
468|voidlinux|2.68|5.15.11|user||

1) use additional parameter <code>--platform=linux/s390x</code><br>
2) use additional parameter <code>--platform=linux/386</code><br>

## Linux distribution dependency tree

```
alpine
altlinux
archlinux ---> manjaro ---> biglinux
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
