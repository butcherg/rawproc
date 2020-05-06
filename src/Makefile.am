AUTOMAKE_OPTIONS = -Wno-portability

bdate="$(shell date)"
AM_CPPFLAGS=-DBUILDDATE='$(bdate)'

nobase_include_HEADERS = gimage/gimage.h gimage/curve.h gimage/half.hpp gimage/strutil.h
EXTRA_DIST = checked.xpm unchecked.xpm thumb_down.xpm thumb_up.xpm run.xpm undo.xpm 

bin_PROGRAMS = rawproc img exif wxcmd

rawproc_SOURCES =  curve.cpp CurvePane.cpp CameraData.cpp elapsedtime.cpp gimage.cpp gimage_cmd.cpp gimage_parse.cpp gimage_process.cpp jpegexif.cpp jpegimage.cpp myListCtrl.cpp myListDialog.cpp myBatchDialog.cpp myConfig.cpp myDoubleSlider.cpp myEXIFDialog.cpp myHistogramDialog.cpp myHistogramPane.cpp myPropertyDialog.cpp myRowSizer.cpp myRowColumnSizer.cpp myFloatCtrl.cpp myIntegerCtrl.cpp myToneCurvePane.cpp PicPanel.cpp PicProcessorCACorrect.cpp PicProcessorHLRecover.cpp PicProcessorGroup.cpp PicProcessorSubtract.cpp PicProcessorDemosaic.cpp PicProcessorBlackWhitePoint.cpp PicProcessorColorSpace.cpp PicProcessor.cpp PicProcessorCrop.cpp PicProcessorCurve.cpp PicProcessorDenoise.cpp PicProcessorExposure.cpp PicProcessorTone.cpp PicProcessorGray.cpp PicProcessorRedEye.cpp PicProcessorResize.cpp PicProcessorRotate.cpp PicProcessorSaturation.cpp PicProcessorSharpen.cpp PicProcessorWhiteBalance.cpp PicProcPanel.cpp pngimage.cpp rawimage.cpp rawprocApp.cpp rawprocFrm.cpp strutil.cpp fileutil.cpp tiffimage.cpp util.cpp cJSON.c 

rawproc_SOURCES += CurvePane.h CameraData.h elapsedtime.h gimage_cmd.h gimage_parse.h gimage_process_h jpegexif.h jpegimage.h myConfig.h myDoubleSlider.h myListCtrl.h myListDialog.h myBatchDialog.h myEXIFDialog.h myHistogramDialog.h myHistogramPane.h myPropertyDialog.h myRowSizer.h myRowColumnSizer.h myFloatCtrl.h myIntegerCtrl.h myToneCurvePane.h PicPanel.h PicProcessorCACorrect.h PicProcessorHLRecover.h PicProcessorGroup.h PicProcessorSubtract.h PicProcessorDemosaic.h PicProcessorBlackWhitePoint.h PicProcessorColorSpace.h  PicProcessor.h PicProcessorCrop.h PicProcessorCurve.h PicProcessorDenoise.h PicProcessorExposure.h PicProcessorTone.h PicProcessorGray.h  PicProcessorRedEye.h PicProcessorResize.h PicProcessorRotate.h PicProcessorSaturation.h PicProcessorSharpen.h PicProcessorWhiteBalance.h PicProcPanel.h pngimage.h rawimage.h rawprocApp.h rawprocFrm.h tiffimage.h util.h nikonlensid.h cJSON.h fileutil.h

if LENSFUN
rawproc_SOURCES += PicProcessorLensCorrection.cpp PicProcessorLensCorrection.h 
if LENSFUNUPDATE
rawproc_SOURCES += lensfun_dbupdate.cpp lensfun_dbupdate.h
endif
endif



rawproc_LDADD = @WX_LIBS@

wxcmd_SOURCES = wxcmd.cpp

wxcmd_LDADD = @WX_LIBS@

img_SOURCES = img.cpp curve.cpp CameraData.cpp elapsedtime.cpp gimage.cpp gimage_cmd.cpp gimage_parse.cpp gimage_process.cpp jpegexif.cpp jpegimage.cpp myConfig.cpp pngimage.cpp rawimage.cpp strutil.cpp fileutil.cpp tiffimage.cpp cJSON.c

img_SOURCES += img.h curve.h CameraData.h elapsedtime.h gimage.h gimage_cmd.h gimage_parse.h gimage_process.h jpegexif.h jpegimage.h myConfig.h pngimage.h rawimage.h strutil.h fileutil.h tiffimage.h cJSON.h

exif_SOURCES = exif.cpp curve.cpp elapsedtime.cpp gimage.cpp jpegexif.cpp jpegimage.cpp myConfig.cpp pngimage.cpp rawimage.cpp strutil.cpp fileutil.cpp tiffimage.cpp cJSON.c

exif_SOURCES += exif.h curve.h elapsedtime.h gimage.h jpegexif.h jpegimage.h myConfig.h pngimage.h rawimage.h strutil.h fileutil.h tiffimage.h cJSON.h


