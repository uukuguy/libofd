# libofd
The first open-source C++ development library for OFD.

"Electronic files storage and exchange formats - Fixed layout documents", referred to as OFD (Open Fixed-Layout Document), is China's independent research and development of fixed-layout format standard, is an important part of the national standard system of electronic documents.  

October 13, 2016, the official release of the OFDnational standard, the standard number: 33190-2016. GB/T standard query site http://www.sac.gov.cn/.

The libofd is the first open-source C++ development library for OFD. It was established in October 20, 2016, only 1 week later than the standard release time. 

## Release Notes

### v0.6
2017.01.09

- ofdviewer cant view ofd file, including simple navigate by up/down arrow.
- Use command "make sample0" to build ./output/sample0.ofd from ./data/sample0.pdf.
- Use command "make sample2" to build ./output/sample2.ofd from ./data/sample2.pdf.
- Use command "make view0" to view ./output/sample0.ofd by ofdviewer.
- Use command "make view2" to view ./output/sample2.ofd by ofdviewer.

### v0.5

2016.12.16

- Complete OFD format file package.
- pdf2ofd can convernt PDF file into OFD file, include text only.
- use command "pdf2ofd <pdffile> [ofdfile]" to translate a PDF file into OFD file.
- Use command "make pdf2ofd" to build library(libofd) and tool(pdf2ofd)
- Use command "make check" to check files in package is meet the OFD standard.

## Objectives

### v1.0

- Read and write OFD files.
- Import text, path and image from PDF files.
- Render text, path and image in OFD files.

## Tools

### pdf2ofd

pdf2ofd is a tool translating a PDF file to a OFD file.

### ofdviewer


## Dependencies

### fontforge

```
$ sudo add-apt-repository ppa:fontforge/fontforge
$ sudo apt-get update
$ sudo apt-get insatll fontforge
```
### libsdl

- libsdl2-dev
- libsdl2-image-dev
- libsdl2-ttf-dev
- libsdl2-mixer-dev
- libsdl2-net-dev
- libsdl2-gfx-dev

### freetype

- libfreetype6-dev
- libharfbuzz-dev
- libicu-dev
- libcairo2-dev http://cairographics.org
- libzip-dev https://github.com/nih-at/libzip 
- easylogging++ https://github.com/easylogging/easyloggingpp


### poppler

```
$ sudo apt-get install libpoppler-private-dev
```

## Build

```
$ git clone https://github.com/idleuncle/libofd.git
```

```
$ make build
```

or

```
$ cd libofd
$ mkdir build && cd build && cmake .. && make
```

## Test

```
$ make pdf2ofd

$ make ofdviewer
```

or 

```
$ build/bin/pdf2ofd data/sample0.pdf

$ build/bin/ofdviewer --v=1 sample0.ofd
```
The command above will create a OFD file named sampl0.ofd in current directory, and upzip it into sample0 folder. See Makefile for detail.

## Install fontforge on Mac OSX with Hombrew

```
$ brew install python gettext libpng jpeg libtiff giflib cairo pango libspiro czmq fontconfig automake libtool pkg-config glib pango
$ brew install -v --debug --with-giflib --with-libspiro fontforge

# Install fontforge UI
$ brew tap caskroom/cask
$ brew cask install fontforge
```

## Errata

- What's the differences between PublicRes and DocumentRes?

- Section 6.2 P4 图中表明每个Doc\_N只有一个PublicRes以及一个DocumentRes，而Section 7.5 P10 表6中PublicRes和DocumentRes的说明明确两者都是“资源序列，每个节点指向OFD包内的一个资源描述文档“，但它们的类型都是ST\_Loc。矛盾的情况下，libofd按结构图中的定义，只允许一个PublicRes、一个DocumentRes。页面中的PageRes也有同样的问题。

- Section 7.4 P8 表4第一行DocID备注“可选”，按附录A.1 OFD.xsd中P92定义应为“必选”。

- Section 8.3.2 P32 表26最后一行“格构高洛德渐变”应为“网格高洛德渐变”

- Section 8.2.1 P28 "当图元未定义绘制属性时，应首先按照图元定义的绘制参数进行渲染"该句重复，应删除。
