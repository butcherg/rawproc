

#add_library(gimage OBJECT gimage.cpp curve.cpp jpegexif.cpp jpegimage.cpp pngimage.cpp rawimage.cpp tiffimage.cpp)

#add_library(gimage_proc OBJECT gimage_cmd.cpp gimage_parse.cpp gimage_process.cpp)

#add_library(util OBJECT strutil.cpp fileutil.cpp cJSON.c myConfig.cpp elapsedtime.cpp CameraData.cpp)

target_sources(rawproc PUBLIC
	rawprocApp.cpp rawprocFrm.cpp CurvePane.cpp  myListCtrl.cpp myListDialog.cpp myDataDialog.cpp myBatchDialog.cpp myDoubleSlider.cpp myEXIFDialog.cpp myHistogramDialog.cpp myHistogramPane.cpp myPropertyDialog.cpp myMetadataDialog.cpp myRowSizer.cpp myRowColumnSizer.cpp myFloatCtrl.cpp myIntegerCtrl.cpp myToneCurvePane.cpp PicPanel.cpp PicProcessorScript.cpp PicProcessorAdd.cpp PicProcessorCACorrect.cpp PicProcessorHLRecover.cpp PicProcessorGroup.cpp PicProcessorSubtract.cpp PicProcessorDemosaic.cpp PicProcessorBlackWhitePoint.cpp PicProcessorColorSpace.cpp PicProcessor.cpp PicProcessorCrop.cpp PicProcessorCurve.cpp PicProcessorDenoise.cpp PicProcessorExposure.cpp PicProcessorTone.cpp PicProcessorGray.cpp PicProcessorLensCorrection.cpp  PicProcessorLensDistortion.cpp PicProcessorLensVignetting.cpp PicProcessorRedEye.cpp PicProcessorResize.cpp PicProcessorRotate.cpp PicProcessorSaturation.cpp PicProcessorSharpen.cpp PicProcessorSpot.cpp PicProcessorWhiteBalance.cpp PicProcPanel.cpp   util.cpp gimage.cpp curve.cpp jpegexif.cpp jpegimage.cpp pngimage.cpp rawimage.cpp tiffimage.cpp gimage_cmd.cpp gimage_parse.cpp gimage_process.cpp strutil.cpp fileutil.cpp cJSON.c myConfig.cpp elapsedtime.cpp CameraData.cpp
	)

target_sources(img PUBLIC
	img.cpp  gimage.cpp curve.cpp jpegexif.cpp jpegimage.cpp pngimage.cpp rawimage.cpp tiffimage.cpp gimage_cmd.cpp gimage_parse.cpp gimage_process.cpp strutil.cpp fileutil.cpp cJSON.c myConfig.cpp elapsedtime.cpp CameraData.cpp
	)

target_sources(wxcmd PUBLIC
	wxcmd.cpp
	)
	
target_sources(exif PUBLIC
	exif.cpp gimage.cpp curve.cpp jpegexif.cpp jpegimage.cpp pngimage.cpp rawimage.cpp tiffimage.cpp gimage_cmd.cpp gimage_parse.cpp gimage_process.cpp strutil.cpp fileutil.cpp cJSON.c myConfig.cpp elapsedtime.cpp CameraData.cpp
	)
	
if(GMIC OR BUILD_GMIC)
	target_sources(rawproc PUBLIC
		PicProcessorGMIC.cpp PicProcessorGMIC.h
	)
endif(GMIC OR BUILD_GMIC)

if(LENSFUN_DBUPDATE)
	target_sources(rawproc PUBLIC
		lensfun_dbupdate.cpp
	)
endif(LENSFUN_DBUPDATE)

#target_include_directories(gimage PUBLIC "${CMAKE_CURRENT_BINARY_DIR}/external/usr/local/include")


target_include_directories(rawproc PRIVATE ".")
target_include_directories(rawproc PRIVATE "gimage")

target_include_directories(img PRIVATE ".")
target_include_directories(img PRIVATE "gimage")
