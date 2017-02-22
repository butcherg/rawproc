# rawproc
Raw camera and general image processor, my way...

I wanted a simple raw processor.  Here it is.  It's primarily a desktop GUI application, but I'm aiming to make it tablet-
friendly.  It doesn't do image management. With the exception of saturation, denoise, and sharpen tools, it only does tone manipulation.  It saves 
the processing applied in the EXIF of the saved image because I do not like the sidecar concept.  It works internally 
with floating point pixel values.  Here's a list of the implemented manipulations:

- Black/White Point
- Bright
- Contrast
- Crop
- Curve
- Denoise
- Gamma
- Gray
- Highlight
- Resize
- Saturation
- Shadow
- Sharpen

You open an image, add whatever manipulations you want to apply to the list, then save the result.   You determine
the order of the manipulations.  Also, you have to deal with the raw image, in that the choices regarding white balance,
demosaic, colorspace conversion, and gamma are for you to make.  The input image is dark, so learn to deal with it.  
And, in doing so, you'll learn valuable things about digital images.

In the display, you can use the 't' key or double-click the upper-left thumbnail to toggle between a small repeat image for panning, 
a 255-value histogram, and no thumbnail.

So, you can open a RAW file, apply a list of manipulations, and save it as, say, a TIFF.  Then, using the "Open Source..." menu item,  
you can select the saved TIFF, and rawproc will open the original RAW file and automatically apply the manipulation list 
saved in the TIFF.  The manipulations used to produce the TIFF are stored in its EXIF metadata.  This is my take on 'non-destructive' editing.

rawproc also implements what I call 'incremental processing', in that each added manipulation does its thing against the 
previous one and stores the result; adding a manipulation doesn't restart the whole processing chain, it just pulls the 
previous processed image and applies its manipulation.  If you go back and change one, it only has to process from that 
point forward.  This approach uses more memory, but at the benefit of less time to see the results of single manipulations.

The check box by each manipulation sets the display to that manipulation's result.  So, you can set the display and go to 
any previous (or subsequent) manipulation and play with it, seeing the result at the checked manipulation.  A kinda goofy
way to display, but I'm warming to it... :)  Crop is the exception to this behavior; if you want to see the actual cropped
image, you have to check a subsequent manipulation.

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
</ul>

I started rawproc development with FreeImage, http://freeimage.sourceforge.net/.  It served well to flesh out 
the initial look and behavior of rawproc, but I encounterd significant hurdles with it and color management.  So, 
I ended up writing my own image library, gimage, https://github.com/butcherg/gimage.  That was a significant learning 
endeavor, but well worth the effort, as I now have high-quality image algorithms with OpenMP threading throughout, with 
color management tools to boot.

Color management is new to 0.5, and is probably deficient in more than one way.  I defer to LibRaw for input profiles and
transformations, and use Little CMS for display and output transforms.  I also made use of Elle Stone's profile code to
implement internal profiles.  Any feedback on correcting or improving what I've implemented for color 
management is appreciated.

I offer no promise of support or enhancement, past offering the code on Github for you to fork. It is organized to compile 
with the GNU g++ compiler, either native on Linux OSs or Mingw32 on Windows platforms.  I've compiled and run executables 
on Ubuntu x86_64 and Windows 7, 8, and 10.

To compile in a Linux OS,  you need to have wxWidgets, gimage, LibRaw, libjpeg, libtiff, and LittleCMS installed, with the associated development headers.  
Basically the same thing applies to compiling in Windows, except you'll probably have to spend more time figuring where 
you put wxWidgets.  The wx-config utility that comes with wxWidgets helps substantially; I ran:

wx-config --libs --cxxflags

and copied the results to Makefile.txt.  You apparently should be able to run wx-config in the Makefile script, but I 
couldn't get it to work right in the Windows/Mingw32 environment.  Once you have the Makefile.txt set up properly, 
you should be able to run 'make -f Makefile.txt' in the directory containing the rawproc source code.  Oh, make sure
you make a 'linux' or 'win' subdirectory in the rawproc directory to hold the object and executable files.

A late add is 'img', a command line program that uses the same image processing routines as rawproc. The only 
comprehensive documentation is the img.cpp source file.  You use it like this:

<pre>
img input.jpg resize:640,0 sharpen:1 output.jpg
</pre>

img will allow wildcards in the input and output filespecifications, so you can use it to apply processing to all 
images in a directory.

This code is essentially a hack; I started it with a wxDevC++ project, but abandoned that IDE some time ago.  I wrote code 
for things I could understand; and shamelessly copied code (e.g., spline.h) for things I didn't want to spend the time 
learning.  My C++ skills are spotty, and look a lot more like C in some places.  But I learned a ton about digital imaging
doing this, and I now have a tool I can use in the field to do what I'll call 'contact sheet' processing; at a 
later time I can go back to the JPEG I produced and extract the manipulation list, use it as the start for more 'quality' 
processing in Raw Therapee and GIMP.

If  you want to gripe or comment about rawproc, I'll be occasionally monitoring the pixels.us forums.  If I subsequently 
commit anything interesting to the respository, I'll shout it out there.
