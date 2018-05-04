AM_CPPFLAGS=-DBUILDDATE=foo

bin_PROGRAMS = rawproc img exif

rawproc_SOURCES = curve.cpp CurvePane.cpp elapsedtime.cpp gimage.cpp jpegexif.cpp jpegimage.cpp myConfig.cpp myDoubleSlider.cpp myEXIFDialog.cpp myHistogramDialog.cpp myHistogramPane.cpp myPropertyDialog.cpp PicPanel.cpp PicProcessorBlackWhitePoint.cpp PicProcessorBright.cpp PicProcessorColorSpace.cpp PicProcessorContrast.cpp PicProcessor.cpp PicProcessorCrop.cpp PicProcessorCurve.cpp PicProcessorDenoise.cpp PicProcessorExposure.cpp PicProcessorGamma.cpp PicProcessorGray.cpp PicProcessorHighlight.cpp  PicProcessorRedEye.cpp PicProcessorResize.cpp PicProcessorRotate.cpp PicProcessorSaturation.cpp PicProcessorShadow.cpp PicProcessorSharpen.cpp PicProcPanel.cpp pngimage.cpp rawimage.cpp rawprocApp.cpp rawprocFrm.cpp strutil.cpp tiffimage.cpp util.cpp

if LENSFUN
rawproc_SOURCES += PicProcessorLensCorrection.cpp
endif

rawproc_LDADD = @LIBS@
rawproc_CXXFLAGS = @CXXFLAGS@

img_SOURCES = img.cpp curve.cpp elapsedtime.cpp gimage.cpp jpegexif.cpp jpegimage.cpp myConfig.cpp pngimage.cpp rawimage.cpp strutil.cpp tiffimage.cpp
img_LDADD = @LIBS@
img_CXXFLAGS = @CXXFLAGS@

exif_SOURCES = exif.cpp curve.cpp elapsedtime.cpp gimage.cpp jpegexif.cpp jpegimage.cpp myConfig.cpp pngimage.cpp rawimage.cpp strutil.cpp tiffimage.cpp
exif_LDADD = @LIBS@
exif_CXXFLAGS = @CXXFLAGS@
