AUTOMAKE_OPTIONS = -Wno-portability

bdate="$(shell date)"
AM_CPPFLAGS=-DBUILDDATE='$(bdate)'

nobase_include_HEADERS = gimage/gimage.h gimage/curve.h gimage/half.hpp gimage/strutil.h
EXTRA_DIST = checked.xpm unchecked.xpm thumb_down.xpm thumb_up.xpm run.xpm undo.xpm 

bin_PROGRAMS = rawproc img exif

rawproc_SOURCES =  curve.cpp CurvePane.cpp elapsedtime.cpp gimage.cpp jpegexif.cpp jpegimage.cpp myConfig.cpp myDoubleSlider.cpp myEXIFDialog.cpp myHistogramDialog.cpp myHistogramPane.cpp myPropertyDialog.cpp myRowColumnSizer.cpp PicPanel.cpp PicProcessorBlackWhitePoint.cpp PicProcessorBright.cpp PicProcessorColorSpace.cpp PicProcessorContrast.cpp PicProcessor.cpp PicProcessorCrop.cpp PicProcessorCurve.cpp PicProcessorDenoise.cpp PicProcessorExposure.cpp PicProcessorGamma.cpp PicProcessorGray.cpp PicProcessorHighlight.cpp  PicProcessorRedEye.cpp PicProcessorResize.cpp PicProcessorRotate.cpp PicProcessorSaturation.cpp PicProcessorShadow.cpp PicProcessorSharpen.cpp PicProcessorWhiteBalance.cpp PicProcPanel.cpp pngimage.cpp rawimage.cpp rawprocApp.cpp rawprocFrm.cpp strutil.cpp tiffimage.cpp util.cpp cJSON.c

rawproc_SOURCES += CurvePane.h elapsedtime.h jpegexif.h jpegimage.h myConfig.h myDoubleSlider.h myEXIFDialog.h myHistogramDialog.h myHistogramPane.h myPropertyDialog.h myRowColumnSizer.h PicPanel.h PicProcessorBlackWhitePoint.h PicProcessorBright.h PicProcessorColorSpace.h PicProcessorContrast.h PicProcessor.h PicProcessorCrop.h PicProcessorCurve.h PicProcessorDenoise.h PicProcessorExposure.h PicProcessorGamma.h PicProcessorGray.h PicProcessorHighlight.h  PicProcessorRedEye.h PicProcessorResize.h PicProcessorRotate.h PicProcessorSaturation.h PicProcessorShadow.h PicProcessorSharpen.h PicProcessorWhiteBalance.h PicProcPanel.h pngimage.h rawimage.h rawprocApp.h rawprocFrm.h tiffimage.h util.h nikonlensid.h cJSON.h

if LENSFUN
rawproc_SOURCES += PicProcessorLensCorrection.cpp PicProcessorLensCorrection.h
endif

if DEMOSAIC
rawproc_SOURCES += PicProcessorDemosaic.cpp PicProcessorDemosaic.h
endif

rawproc_LDADD = @WX_LIBS@


img_SOURCES = img.cpp curve.cpp elapsedtime.cpp gimage.cpp jpegexif.cpp jpegimage.cpp myConfig.cpp pngimage.cpp rawimage.cpp strutil.cpp tiffimage.cpp cJSON.c

exif_SOURCES = exif.cpp curve.cpp elapsedtime.cpp gimage.cpp jpegexif.cpp jpegimage.cpp myConfig.cpp pngimage.cpp rawimage.cpp strutil.cpp tiffimage.cpp cJSON.c

