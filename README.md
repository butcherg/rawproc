# rawproc
Raw processor and general image processor.

I wanted a simple raw processor.  Here it is.  It's primarily a desktop GUI application, but one day I'll make it tablet-
friendly (well maybe, here in 2023 less enamored with the idea).  It doesn't do image management. There are no masking tools
to localize processing.  No fancy color tools.  It saves the processing applied in the EXIF of the saved image because I 
do not like the sidecar concept.  It works internally with floating point pixel values.  Here's a list of the implemented 
manipulations:

- Add
- Black/White Point
- Colorspace
- Crop
- Curve
- Demosaic
- Denoise
- Exposure
- Gamma
- Grayscale Conversion
- G'MIC (optional, specified at build time)
- Highlight Recovery
- Lens Correction
- Lens Distortion (enabled in the configuration properties)
- Lens Vignetting (enabled in the configuration properties)
- Redeye Correction
- Resize
- Rotate
- Saturation
- Script
- Sharpen
- Subtract
- Tone
- White Balance

You open an image, add whatever manipulations you want to apply to the list, then save the result.   You determine
the order of the manipulations.  Also, you have to deal with the raw image, in that the choices regarding white balance,
demosaic, colorspace conversion, and gamma are for you to make.  The input image is dark, so learn to deal with it.  
And, in doing so, you'll learn valuable things about digital images.

So, you can open a RAW file, apply a list of manipulations, and save it as, say, a TIFF.  Then, using the "Open Source..." 
menu item,  you can select the saved TIFF, and rawproc will open the original RAW file and automatically apply the 
manipulation list saved in the TIFF.  The manipulations used to produce the TIFF are stored in its EXIF metadata. 
This is my take on 'non-destructive' editing.

rawproc also implements what I call 'incremental processing', in that each added manipulation does its thing against the 
previous one and stores the result; adding a manipulation doesn't restart the whole processing chain, it just pulls the 
previous processed image and applies its manipulation.  If you go back and change one, it only has to process from that 
point forward.  This approach uses more memory, but at the benefit of less time to see the results of single manipulations.

The check box by each manipulation sets the display to that manipulation's result.  So, you can set the display and go to 
any previous (or subsequent) manipulation and play with it, seeing the result at the checked manipulation.  A kinda goofy
way to display, but I'm warming to it... :)  

This code is licensed for widespread use under the terms of the GPL, Version 2.

Contributed code and algorithmns:
<ul>
	<li>Spline Curve: Copyright (C) 2011, 2014 Tino Kluge, http://kluge.in-chemnitz.de/opensource/spline/, GPL2</li>
	<li>Saturation: adapted from a public-domain function by Darel Rex Finley, http://alienryderflex.com/saturation.html</li>
	<li>Non-Local Means Denoise: A. Buades, http://www.ipol.im/pub/art/2011/bcm_nlm/; adapted from from an interpretation by
		David Tschumperlé presented at http://opensource.graphics/, 17 Sep 2015, #4.</li>
	<li>Rotation: Alan Paeth, "A Fast Algorithm for General Raster Rotation", Graphics Interface '86
	  http://supercomputingblog.com/graphics/coding-bilinear-interpolation/, and Coding Bilinear Interpolation, 
	  http://supercomputingblog.com/graphics/coding-bilinear-interpolation/</li>
	<li>Resizing: Graphics Gems III: Schumacher, Dale A., General Filtered Image Rescaling, p. 8-16
	  https://github.com/erich666/GraphicsGems/blob/master/gemsiii/filter.c</li>
	<li>Color Management: Primaries and black/white points from Elle Stone's make-elles-profiles.c, 
	  https://github.com/ellelstone/elles_icc_profiles</li>
	<li>Demosaic: librtprocess, https://github.com/CarVac/librtprocess</li>
</ul>

I started rawproc development with FreeImage, http://freeimage.sourceforge.net/.  It served well to flesh out 
the initial look and behavior of rawproc, but I encounterd significant hurdles with it and color management.  So, 
I ended up writing my own image library, gimage, https://github.com/butcherg/gimage.  That was a significant learning 
endeavor, but well worth the effort, as I now have high-quality image algorithms with OpenMP threading throughout, with 
color management tools to boot.  Also, gimage has only one internal tone representation, floating point.  Image
manipulations are a little slower as a result, but I've done some pixel-peep comparisons and the tone gradations do
look better.  double, float, or half floating point representations are selectable at compile-time.  I compile my Linux 
version with float; I can't see that using double provides any advantage worth the memory use for the tone ranges we 
work with in general purpose photography.

I offer no promise of support or enhancement, past offering the code on Github for you to fork. It is organized to compile 
with the GNU g++ compiler, either native on Linux OSs or Mingw32 on Windows platforms.  I've compiled and run executables 
on Ubuntu x86_64 and Windows 7, 8, and 10.

'img' is a command line program that uses the same image processing routines as rawproc. The only comprehensive 
documentation is the img.cpp source file.  You use it like this:

<pre>
img input.jpg resize:640,0 sharpen:1 output.jpg
</pre>

img will allow wildcards in the input and output filespecifications, so you can use it to apply processing to all 
images in a directory.  It will also read rawproc configuration files, so a workflow that starts with batch-produced JPEGs and 
continues with re-editing selected images in rawproc is seamless.

This code is essentially a hack; I started it with a wxDevC++ project, but abandoned that IDE some time ago.  I wrote code 
for things I could understand; and shamelessly copied code (e.g., spline.h) for things I didn't want to spend the time 
learning.  My C++ skills are spotty, and look a lot more like C in some places.  But I learned a ton about digital imaging
doing this, and I now have a tool I can use in the field to do what I'll call 'contact sheet' processing; at a 
later time I can go back to the JPEG I produced and extract the manipulation list, use it as the start for more 'quality' 
processing in Raw Therapee and GIMP.

If  you want to gripe or comment about rawproc, I'll be occasionally monitoring the discuss.pixls.us forums.  If I subsequently 
commit anything interesting to the respository, I'll shout it out there.

# Building rawproc

For rawproc 1.4, the cmake build system has been significantly revamped.  It is now possible to compile rawproc with all supporting packages 
supplied by the operating system.  However, that approach will yield a somewhat less-than-interesting rawproc, so this missive will also cover
the new cmake build system's ability to retrive, compile, and install almost all of the rawproc dependencies.  Most of this guidance will be
illustrated on a Linux (Debian/Ubuntu) system; a Windows build using MSYS2 will also be covered.

## Minimal Build

First, current rawproc requires a C++17 compiler. gcc 11 uses it by default; gcc >= 8 requires the -std=C++17 switch.   gcc is extensively tested;  clang has not been tested but would probably work. 

In order to build a basic rawproc, you need to install the following:

<pre>
sudo apt-get install libjpeg-dev libtiff-dev libpng-dev libwxgtk3.0-gtk3-dev liblcms2-dev libraw-dev liblensfun-dev libexiv2-dev
</pre>

and then, starting in the rawproc directory:

<pre>
$ mkdir build
$ cd build
$ cmake ..
$ make
$ make conf
$ make docpages
$ make doc
</pre>

make conf builds a rawproc.conf file from the defaults for the configuration properties.  make docpages pulls relevant data from the source
code for the help file pages.  make doc assembles a help file for rawproc, access through Help->View Help.  If you don't build the help file,
rawproc will complain about not having it each time it starts.

That's it.  You'll now have a version of rawproc that'll open a wide variety of raw formats, process them to a pleasing rendition, and save 
the rendition to either a JPEG, TIFF, or PNG image file.

However, that build will be missing some significant capabilities that comprise a functional raw processor.  Particularly, the only demosaic
operation available is half, a simple consolidation of channels that forms a half-sized image.  And, while a lens correction tool is 
available in the operations menu, no database of camera correction data will be available.  So, let's correct that with a better build...

## Functional Build

First, install these additional prerequisites for lensfun database updating:

<pre>
sudo apt-get libcurl4-openssl-dev libarchive-dev
</pre>

Next, re-configure the build as follows, starting in the build directory:

<pre>
$ cmake -DBUILD_LIBRTPROCESS=GITHUB -DLENSFUN_DBUPDATE=ON ..
$ make
</pre>

This time, the make will clone, build, and install the librtprocess library of demosaic routines.  And, when rawproc compiles, it'll include
the capability to install and update the appropriate lensfun camera correction database.  To install the lensfun database, run the new 
rawproc executable, then in the menu do Edit->Data Update.  You can check the database version and currency in the Help->About dialog.

Note: Lensfun database update is disabled by default, I decided a default rawproc should not include code to connect to the internet.

Now, if you're okay with the currency and capabilities of the distro-installed libraries, this build will fully serve your raw processing needs.

## The Full Build System

Keeping a raw processor current with new cameras can be vexing.  Also, the libraries go through various evolutions, some which are compelling
to have.  Me, in order to deliver fully-capable release builds, I wanted to use the Github master branches for the major libraries.  The
cmake build system now accomoodates this by exposing BUILD_ switches that work like this:

1. -DBUILD_(library)=GITHUB clones, builds, and installs the specified library for which rawproc to link.
2. -DBUILD_(library)=SRCPKG will download a compatible source tar.gz from the library release page, then build and install it for rawproc linking.
3. -DBUILD_(library)=/path/to/source.tar.gz unpacks, builds and installs the library from a tar.gz file in a local directory.

The following BUILD_ switches are available, each able to perform all three switch options:

<pre>
BUILD_LCMS2
BUILD_LIBRAW
BUILD_LENSFUN
BUILD_EXIV2
BUILD_LIBRTPROCESS
BUILD_GMIC
</pre>

If a BUILD_ switch is selected for a library, the library will be compiled to a .a file for static linking and installed to a lib/include/share 
location under the build directory at external/usr.

There's a bit of a downside to using a BUILD_ switch, that being the need to manually specify link dependencies.  Since I couldn't figure out
how to cleanly use cmake's native package search and dependency capabillties to handle a locally-installed static library, you may have to pass 
dependent libraries in your cmake command.  To do this, the build system exposes _DEPS switches for each BUILD_ switch, e.g. LIBRAW_DEPS
for BUILD_LIBRAW.  For example, lensfun needs glib2, so a cmake command that downloads the lensfun source package would look like this:

<pre>
$ cmake -DBUILD_LENSFUN=SRCPKG -DLENSFUN_DEPS=glib-2.0 ..
</pre>

Note that dependencies are identified by the name that would be specified on the link command line, e.g., -lglib-2.0, but without the '-l'.  
If multiple libraries need to be provided, separate them with semi-colons, e.g., glib-2.0;intl;iconv - this complies with how cmake handles 
lists of things.

Finally, there are two optional libraries, librtprocess and G'MIC.  If you specify either -DBUILD_GMIC=... or -DGMIC=ON, libgmic will be 
incorporated into rawproc, the latter switch will search for a system installation of libgmic.  Same for librtprocess, either 
-DBUILD_LIBRTPROCESS=... or -DLIBRTPROCESS=ON.

## Windows Builds

First, the guidance provided above applies to rawproc builds using MSYS2.  With that, the prerequistes are:

<pre>
$ pacman -S mingw-w64-x86_64-wxWidgets mingw-w64-x86_64-lcms2 mingw-w64-x86_64-libraw mingw-w64-x86_64-lensfun mingw-w64-x86_64-exiv2 libcurl-devel libarchive 
</pre>

librtprocess and libgmic are not currently available from the MSYS2 package repository, so you'll have to use the BUILD_ switches.

## Build Notes

- The cmake build system does not yet provide well-considered install targets.
- There are other switches that support a mxe cross-compile build, but I'm not going to document their use.
- wxWidgets isn't supported by the BUILD_ concept; you either have to install a distro package or clone/download,build,compile wxWidgets in a separate source tree.  If you do the latter, you can point to it in the cmake command line with -DWXDIR=/path/to/wxwidgets_build_directory_containing_wx-config and the cmake build system will do the rest.
