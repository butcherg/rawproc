#Makefile using Codeblocks MinGW and wxWidgets 3.0.2

UNAME := $(shell g++ -dumpmachine)

$(info $(UNAME))

OBJDIR=build

WINFREEIMAGE=c:\FreeImage-3.17.0.ggb
WINWXWIDGETS=c:\wxWidgets-3.0.2
WINLCMS=C:\lcms2-2.8

LIBJPEG=../gimage/jpeg-6b
LIBTIFF=../gimage/tiff-4.0.6
LIBRAW=../gimage/LibRaw-0.17.2
LIBGIMAGE=../gimage

OPENMP=-fopenmp

ifeq ($(UNAME), mingw32)
	WXLIBS= -mthreads -L. -L$(WINWXWIDGETS)\lib\gcc_lib -lwxmsw30u_xrc -lwxmsw30u_aui -lwxmsw30u_html -lwxmsw30u_adv -lwxmsw30u_core -lwxbase30u_xml -lwxbase30u_net -lwxbase30u -lwxtiff -lwxjpeg -lwxpng -lwxzlib -lwxregexu -lwxexpat -lkernel32 -luser32 -lgdi32 -lcomdlg32 -lwxregexu -lwinspool -lwinmm -lshell32 -lcomctl32 -lole32 -loleaut32 -luuid -lrpcrt4 -ladvapi32 -lwsock32
	WXFLAGS=-Wl,--subsystem,windows -mwindows -mthreads -DHAVE_W32API_H -D__WXMSW__ -D__WXDEBUG__ -D_UNICODE -I$(WINWXWIDGETS)\lib\gcc_lib\mswu -I$(WINWXWIDGETS)\include -I$(WINFREEIMAGE) -Wno-ctor-dtor-privacy -pipe -fmessage-length=0 
endif

ifeq ($(UNAME), x86_64-linux-gnu)
	WXLIBS=$(shell wx-config --libs std,aui)
	WXFLAGS=$(shell wx-config --cxxflags)
endif

#OBJECTS := $(addprefix $(OBJDIR)/,util.o elapsedtime.o myHistogramPane.o myFileSelector.o CurvePane.o PicProcessorBlackWhitePoint.o PicProcessorHighlight.o PicProcessorShadow.o PicProcessorCurve.o PicProcessorGamma.o PicProcessorBright.o PicProcessorContrast.o PicProcessorSaturation.o PicProcessorGray.o PicProcessorCrop.o PicProcessorSharpen.o PicProcessorResize.o PicProcessorRotate.o PicProcessorDenoise.o PicProcessor.o rawprocFrm.o rawprocApp.o PicProcPanel.o PicPanel.o)

OBJECTS := $(addprefix $(OBJDIR)/,util.o elapsedtime.o  myFileSelector.o CurvePane.o PicProcessor.o PicProcessorCurve.o rawprocFrm.o rawprocApp.o PicProcPanel.o PicPanel.o)

#Configure these:
CXX=g++
WINDRES=windres

#WXEXTERNALLIBS=-lwxexpat -lwxjpeg -lwxpng -lwxtiff -lwxzlib -lwxregexu 

LIBDIRS=
LIBS=-lgimage -lraw -ltiff -ljpeg -llcms2 

INCLUDEDIRS=-I$(LIBGIMAGE) 
CFLAGS=-fopenmp -O4
LFLAGS=-fopenmp

-include $(OBJDIR)/localmake.txt

all: rawproc img

#rawproc: $(OBJECTS)
#ifeq ($(OBJDIR),win)
#	$(CXX)  $(WXFLAGS)  $(OBJECTS) $(LIBS)  $(WXLIBS) $(FILIBS) -s -static $(OPENMP) -o $(OBJDIR)/rawproc 
#else
#	$(CXX) $(WXFLAGS)  $(OBJECTS) $(LIBS)  $(WXLIBS)  -s -fopenmp -o $(OBJDIR)/rawproc
#endif

rawproc:  $(OBJECTS)
	$(CXX) $(LFLAGS) $(LIBDIRS) $(OBJECTS)  $(LIBS) $(WXLIBS) -o $(OBJDIR)/rawproc

img: $(OBJDIR)/img.o
	$(CXX) $(OBJDIR)/img.o $(LIBDIRS) $(LIBS) $(LFLAGS) -o $(OBJDIR)/img

$(OBJDIR)/img.o: img.cpp
	$(CXX) $(CFLAGS) $(INCLUDEDIRS) -std=c++11 -c img.cpp -o$@

$(OBJDIR)/elapsedtime.o: elapsedtime.cpp
	$(CXX) -c elapsedtime.cpp  -o$@

$(OBJDIR)/util.o: util.cpp
	$(CXX) $(CFLAGS) $(INCLUDEDIRS) $(WXFLAGS) -c util.cpp  -o$@

$(OBJDIR)/myFileSelector.o: myFileSelector.cpp
	$(CXX) $(CFLAGS) $(INCLUDEDIRS) $(WXFLAGS) -c myFileSelector.cpp  -o$@

#$(OBJDIR)/myHistogramPane.o: myHistogramPane.cpp
#	$(CXX) $(FIFLAGS) $(WXFLAGS) -c myHistogramPane.cpp  -o$@

$(OBJDIR)/CurvePane.o: CurvePane.cpp 
	$(CXX) $(CFLAGS) $(INCLUDEDIRS) $(WXFLAGS) -w -c CurvePane.cpp  -o$@

$(OBJDIR)/PicProcPanel.o: PicProcPanel.cpp
	$(CXX)  $(CFLAGS) $(INCLUDEDIRS) $(WXFLAGS) -c PicProcPanel.cpp  -o$@

$(OBJDIR)/PicPanel.o: PicPanel.cpp
	$(CXX)  $(CFLAGS) $(INCLUDEDIRS) $(WXFLAGS) -c PicPanel.cpp  -o$@

#$(OBJDIR)/PicProcessorBlackWhitePoint.o: PicProcessorBlackWhitePoint.cpp
#	$(CXX)  $(FIFLAGS) $(WXFLAGS) -w -c PicProcessorBlackWhitePoint.cpp    -o$@

#$(OBJDIR)/PicProcessorHighlight.o: PicProcessorHighlight.cpp
#	$(CXX)  $(FIFLAGS) $(WXFLAGS) -w -c PicProcessorHighlight.cpp    -o$@

#$(OBJDIR)/PicProcessorShadow.o: PicProcessorShadow.cpp
#	$(CXX)  $(FIFLAGS) $(WXFLAGS) -w -c PicProcessorShadow.cpp    -o$@

$(OBJDIR)/PicProcessorCurve.o: PicProcessorCurve.cpp
	$(CXX) $(CFLAGS) $(INCLUDEDIRS) $(WXFLAGS)  -c PicProcessorCurve.cpp    -o$@

#$(OBJDIR)/PicProcessorGamma.o: PicProcessorGamma.cpp
#	$(CXX) $(FIFLAGS) $(WXFLAGS) -w -c PicProcessorGamma.cpp    -o$@

#$(OBJDIR)/PicProcessorBright.o: PicProcessorBright.cpp 
#	$(CXX) $(FIFLAGS) $(WXFLAGS) -w -c PicProcessorBright.cpp   -o$@

#$(OBJDIR)/PicProcessorContrast.o: PicProcessorContrast.cpp 
#	$(CXX) $(FIFLAGS) $(WXFLAGS) -w -c PicProcessorContrast.cpp   -o$@

#$(OBJDIR)/PicProcessorSaturation.o: PicProcessorSaturation.cpp 
#	$(CXX) $(FIFLAGS) $(WXFLAGS) -w -c PicProcessorSaturation.cpp   -o$@

#$(OBJDIR)/PicProcessorGray.o: PicProcessorGray.cpp 
#	$(CXX) $(FIFLAGS) $(WXFLAGS) -w -c PicProcessorGray.cpp   -o$@

#$(OBJDIR)/PicProcessorCrop.o: PicProcessorCrop.cpp 
#	$(CXX) $(FIFLAGS) $(WXFLAGS) -w -c PicProcessorCrop.cpp   -o$@

#$(OBJDIR)/PicProcessorResize.o: PicProcessorResize.cpp 
#	$(CXX) $(FIFLAGS) $(WXFLAGS) -w -c PicProcessorResize.cpp   -o$@

#$(OBJDIR)/PicProcessorSharpen.o: PicProcessorSharpen.cpp 
#	$(CXX) $(FIFLAGS) $(WXFLAGS) -w -c PicProcessorSharpen.cpp   -o$@

#$(OBJDIR)/PicProcessorRotate.o: PicProcessorRotate.cpp 
#	$(CXX) $(FIFLAGS) $(WXFLAGS) -w -c PicProcessorRotate.cpp   -o$@

#$(OBJDIR)/PicProcessorDenoise.o: PicProcessorDenoise.cpp 
#	$(CXX) $(FIFLAGS) $(WXFLAGS) -w -c PicProcessorDenoise.cpp   -o$@

$(OBJDIR)/PicProcessor.o: PicProcessor.cpp
	$(CXX) $(CFLAGS) $(INCLUDEDIRS) $(WXFLAGS) -c PicProcessor.cpp -o$@

$(OBJDIR)/rawprocFrm.o: rawprocFrm.cpp
	$(CXX) $(CFLAGS) $(INCLUDEDIRS) $(WXFLAGS) -c rawprocFrm.cpp   -o$@

$(OBJDIR)/rawprocApp.o: rawprocApp.cpp 
	$(CXX) $(CFLAGS) $(INCLUDEDIRS) $(WXFLAGS) -c rawprocApp.cpp   -o$@

$(OBJDIR)/resource.o: resource.rc
ifeq ($(OBJDIR),win)
	windres -c resource.rc -o$@
endif

makedoc:
	zip  -r -j $(OBJDIR)/rawprocdoc.zip doc/*

clean:
	rm -f $(OBJDIR)/*.o $(OBJDIR)/rawproc 

cleandoc:
	rm -f $(OBJDIR)/rawprocdoc.zip



