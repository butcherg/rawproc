



target_sources(rawproc PUBLIC
	curve.cpp CurvePane.cpp CameraData.cpp elapsedtime.cpp gimage.cpp gimage_cmd.cpp gimage_parse.cpp gimage_process.cpp jpegexif.cpp jpegimage.cpp myListCtrl.cpp myListDialog.cpp myDataDialog.cpp myBatchDialog.cpp myConfig.cpp myDoubleSlider.cpp myEXIFDialog.cpp myHistogramDialog.cpp myHistogramPane.cpp myPropertyDialog.cpp myMetadataDialog.cpp myRowSizer.cpp myRowColumnSizer.cpp myFloatCtrl.cpp myIntegerCtrl.cpp myToneCurvePane.cpp PicPanel.cpp PicProcessorScript.cpp PicProcessorAdd.cpp PicProcessorCACorrect.cpp PicProcessorHLRecover.cpp PicProcessorGroup.cpp PicProcessorSubtract.cpp PicProcessorDemosaic.cpp PicProcessorBlackWhitePoint.cpp PicProcessorColorSpace.cpp PicProcessor.cpp PicProcessorCrop.cpp PicProcessorCurve.cpp PicProcessorDenoise.cpp PicProcessorExposure.cpp PicProcessorTone.cpp PicProcessorGray.cpp PicProcessorLensCorrection.cpp PicProcessorRedEye.cpp PicProcessorResize.cpp PicProcessorRotate.cpp PicProcessorSaturation.cpp PicProcessorSharpen.cpp PicProcessorSpot.cpp PicProcessorWhiteBalance.cpp PicProcPanel.cpp pngimage.cpp rawimage.cpp rawprocApp.cpp rawprocFrm.cpp strutil.cpp fileutil.cpp tiffimage.cpp util.cpp cJSON.c 
	)

target_sources(img PUBLIC
	img.cpp curve.cpp CameraData.cpp elapsedtime.cpp gimage.cpp gimage_cmd.cpp gimage_parse.cpp gimage_process.cpp jpegexif.cpp jpegimage.cpp myConfig.cpp pngimage.cpp rawimage.cpp strutil.cpp fileutil.cpp tiffimage.cpp cJSON.c
	)

target_sources(wxcmd PUBLIC
	wxcmd.cpp
	)
	
target_sources(exif PUBLIC
	exif.cpp curve.cpp elapsedtime.cpp gimage.cpp jpegexif.cpp jpegimage.cpp myConfig.cpp pngimage.cpp rawimage.cpp strutil.cpp fileutil.cpp tiffimage.cpp cJSON.c
	)
	
if(GMIC OR (GMICINC AND GMICLIB))
	target_sources(rawproc PUBLIC
		PicProcessorGMIC.cpp PicProcessorGMIC.h
	)
endif(GMIC OR (GMICINC AND GMICLIB))

if(LENSFUN_DBUPDATE)
	target_sources(rawproc PUBLIC
		lensfun_dbupdate.cpp
	)
endif(LENSFUN_DBUPDATE)

target_include_directories(rawproc PRIVATE ".")
target_include_directories(rawproc PRIVATE "gimage")

target_include_directories(img PRIVATE ".")
target_include_directories(img PRIVATE "gimage")
