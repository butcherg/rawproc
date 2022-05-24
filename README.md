# rawproc
Raw processor and general image processor.

I wanted a simple raw processor.  Here it is.  It's primarily a desktop GUI application, but one day I'll make it tablet-
friendly.  It doesn't do image management. With the exception of saturation, denoise, and sharpen tools, it only does 
tone manipulation.  It saves the processing applied in the EXIF of the saved image because I do not like the sidecar 
concept.  It works internally with floating point pixel values.  Here's a list of the implemented manipulations:

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
- G'MIC
- Highlight Recovery
- Lens Correction
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

In the display, you can use the 't' key or double-click the upper-left thumbnail to toggle between a small repeat image for panning, a 255-value histogram, and no thumbnail.

So, you can open a RAW file, apply a list of manipulations, and save it as, say, a TIFF.  Then, using the "Open Source..." menu item,  you can select the saved TIFF, and rawproc will open the original RAW file and automatically apply the manipulation list saved in the TIFF.  The manipulations used to produce the TIFF are stored in its EXIF metadata.  This is my take on 'non-destructive' editing.

rawproc also implements what I call 'incremental processing', in that each added manipulation does its thing against the 
previous one and stores the result; adding a manipulation doesn't restart the whole processing chain, it just pulls the 
previous processed image and applies its manipulation.  If you go back and change one, it only has to process from that 
point forward.  This approach uses more memory, but at the benefit of less time to see the results of single manipulations.

The check box by each manipulation sets the display to that manipulation's result.  So, you can set the display and go to 
any previous (or subsequent) manipulation and play with it, seeing the result at the checked manipulation.  A kinda goofy
way to display, but I'm warming to it... :)  

This code is licensed for widespread use under the terms of the GPL.

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
look better.  double, float, or half floating point representations are selectable at compile-time; the 32-bit Windows 
installer version uses half because I ran into heap limits with a 32-bit executable.  I compile my Linux version with 
float; I can't see that using double provides any advantage worth the memory use for the tone ranges we work with in 
general purpose photography.

I offer no promise of support or enhancement, past offering the code on Github for you to fork. It is organized to compile 
with the GNU g++ compiler, either native on Linux OSs or Mingw32 on Windows platforms.  I've compiled and run executables 
on Ubuntu x86_64 and Windows 7, 8, and 10.

'img' is a command line program that uses the same image processing routines as rawproc. The only comprehensive 
documentation is the img.cpp source file.  You use it like this:

<pre>
img input.jpg resize:640,0 sharpen:1 output.jpg
</pre>

img will allow wildcards in the input and output filespecifications, so you can use it to apply processing to all 
images in a directory.  It will also read rawproc configuration files, so a workflow that starts with batch-produced JPEGs and continues with re-editing selected images in rawproc is seamless.

This code is essentially a hack; I started it with a wxDevC++ project, but abandoned that IDE some time ago.  I wrote code 
for things I could understand; and shamelessly copied code (e.g., spline.h) for things I didn't want to spend the time 
learning.  My C++ skills are spotty, and look a lot more like C in some places.  But I learned a ton about digital imaging
doing this, and I now have a tool I can use in the field to do what I'll call 'contact sheet' processing; at a 
later time I can go back to the JPEG I produced and extract the manipulation list, use it as the start for more 'quality' 
processing in Raw Therapee and GIMP.

If  you want to gripe or comment about rawproc, I'll be occasionally monitoring the pixels.us forums.  If I subsequently 
commit anything interesting to the respository, I'll shout it out there.

# Building a github Checkout

If you clone the github repository, do these things first:

<pre>
aclocal
autoconf
automake --add-missing
</pre>

Now, you have the files to proceed with ./configure... make... etc.

## Prerequisites:

If you're on a Debian/Ubuntu or derivatives, install these packages:

<pre>
sudo apt-get install libjpeg-dev libtiff-dev libpng-dev liblcms2-dev libraw-dev
</pre>

If you enable lensfun:

<pre>
sudo apt-get install liblensfun-dev libcurl4-openssl-dev libarchive-dev
</pre>

If you want to enjoy the fruits of librtprocess, the nascent effort to package the Raw Therapee
demosaic routines, you'll at present need to compile and install librtprocess from a github clone:

https://github.com/CarVac/librtprocess

Instructions to do so are in the librtprocess README.  Once you've done that, you'll be able to use
--enable-librtprocess in rawproc's ./configure.  Otherwise, the demosaic tool will only allow the
internal algorithms half, half_resize, and color.

Next, if you want to compile rawproc, get the wxWidgets sources and do a static compile:

<pre>
wget https://github.com/wxWidgets/wxWidgets/releases/download/v3.1.2/wxWidgets-3.1.2.tar.bz2
tar -xjvf wxWidgets-3.1.2.tar.bz2
cd wxWidgets-3.1.2/
mkdir build-linux
cd build-linux
../configure --disable-shared --enable-unicode
make
make doc
</pre>

You can make install wxWidgets, I find it just as convenient to use their
wx-config script to link in-place. 

## Building rawproc

cd over to the rawproc directory and
do the following:

<pre>
mkdir build-linux
cd build-linux
../configure --enable-lensfun --enable-librtprocess --with-wx-config=/path/to/wxWidgets-3.1.2/build-linux/wx-config CXXFLAGS=-O3
make
sudo make install
</pre>

...and there you go, rawproc, img, and exif binaries will be installed in
/usr/local/bin.

## Other Tasks

If you're building from source in either Linux or MSYS2, you'll also need to build and manually install your configuration and help files.  In the build directory:

<pre>
make docpages
make doc
make conf
sudo cp src/rawprocdoc.zip /usr/local/bin/.
mkdir ~/.rawproc
cp src/rawproc.conf ~/.rawproc/.
</pre>

Right now, rawproc will only look for its help file in the directory containing the executable.  rawproc will look for 
a rawproc.conf in a few places, but the best place to put it is in a ~/.rawproc directory.  Future versions will use that
directory as a default for other stuff, like cms.profilepath and the lensfun database.

## Building img without rawproc

If you just want to build the img command line program, without the wxWidgets library, then do this:

<pre>
mkdir build-linux
cd build-linux
../configure --enable-lensfun --enable-librtprocess --disable-wxwidgets CXXFLAGS=-O3
cd src
make img
</pre>

# Notes

1. Lens correction using the lensfun library requires particular attention to where the lens correction database is stored.
As of Version 1.0Dev, I recommend setting the tool.lenscorrection.databasepath to a suitable place and using the Update Data... menu selection to pull the most recent version from the lensfun server.

2. Color management requires the user to specify a profile directory, and in that directory shall go all profiles used, 
camera, working, and display/output.  rawproc doesn't use the operating system color management facilities.
