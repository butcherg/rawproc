
cmake_minimum_required(VERSION 3.15)

MESSAGE("CMAKE_HOST_SYSTEM_NAME ${CMAKE_HOST_SYSTEM_NAME}")
MESSAGE("CMAKE_HOST_SYSTEM_VERSION ${CMAKE_HOST_SYSTEM_VERSION}")

project(rawproc LANGUAGES CXX C)

string(TIMESTAMP BUILDDATE)

add_compile_options(-ggdb -DVERSION="1.3"  -DBUILDDATE="${BUILDDATE}")

option(BUILD_LCMS2 "download and build littlecms2 instead of using installed package" OFF)
option(BUILD_LENSFUN "download and build lensfun instead of using installed package" OFF)
option(BUILD_LIBRAW "download and build libraw instead of using installed package" OFF)
option(BUILD_EXIV2 "download and build exiv2 instead of using installed package" OFF)
option(BUILD_LIBRTPROCESS "download and build librtprocess instead of using installed package" OFF)
option(LENSFUN_DBUPDATE "enable the code and library links to update the lensfun database from the menu.  Enables a network connection..." OFF)
option(GMIC "enable the code and library links to use G'MIC in img and rawproc.  Requires libgmic to be installed in a system location." OFF)


add_executable(rawproc src/rawprocApp.cpp)
add_executable(img src/img.cpp)
add_executable(wxcmd src/wxcmd.cpp)
add_executable(exif src/exif.cpp)

# make doc:
add_custom_target(doc 	
	COMMAND
		bash -c "zip -v -r -j rawprocdoc.zip \
		${CMAKE_CURRENT_SOURCE_DIR}/doc/*.html \
		${CMAKE_CURRENT_SOURCE_DIR}/doc/*.css \
		${CMAKE_CURRENT_SOURCE_DIR}/doc/*.png \
		${CMAKE_CURRENT_SOURCE_DIR}/doc/*.hh* "
	VERBATIM
)

add_subdirectory(src)

find_package(OpenMP)
if(OpenMP_CXX_FOUND)
    target_link_libraries(rawproc  OpenMP::OpenMP_CXX)
	target_link_libraries(img  OpenMP::OpenMP_CXX)
	target_link_libraries(exif  OpenMP::OpenMP_CXX)
endif()

if(WXDIR)
	execute_process(COMMAND bash -c "${WXDIR}/wx-config --cxxflags" OUTPUT_VARIABLE WX_FLAGS OUTPUT_STRIP_TRAILING_WHITESPACE)
	execute_process(COMMAND bash -c "${WXDIR}/wx-config --libs base core propgrid aui html" OUTPUT_VARIABLE WX_LIBS OUTPUT_STRIP_TRAILING_WHITESPACE )
	list(APPEND CMAKE_CXX_FLAGS "${WX_FLAGS}")
	target_link_libraries(rawproc ${WX_LIBS})
	target_link_libraries(wxcmd ${WX_LIBS})
	target_link_libraries(img ${WX_LIBS})
	target_link_libraries(exif ${WX_LIBS})
	message(STATUS "wxWidgets configured with wx-config at ${WXDIR}")
else()
	find_package(wxWidgets REQUIRED COMPONENTS std aui propgrid)
	message(STATUS "includes: ${wxWidgets_USE_FILE}")
	include(${wxWidgets_USE_FILE})
	target_link_libraries(rawproc ${wxWidgets_LIBRARIES})
	target_link_libraries(wxcmd ${wxWidgets_LIBRARIES})
	target_link_libraries(img ${wxWidgets_LIBRARIES})
	target_link_libraries(exif ${wxWidgets_LIBRARIES})
endif(WXDIR)

find_package(PkgConfig REQUIRED)

find_package(JPEG REQUIRED)
include_directories(${JPEG_INCLUDE_DIR})
target_link_libraries(rawproc ${JPEG_LIBRARIES})
target_link_libraries(img ${JPEG_LIBRARIES})
target_link_libraries(exif ${JPEG_LIBRARIES})

find_package(TIFF REQUIRED)
include_directories(${TIFF_INCLUDE_DIR})
target_link_libraries(rawproc ${TIFF_LIBRARIES})
target_link_libraries(img ${TIFF_LIBRARIES})
target_link_libraries(exif ${TIFF_LIBRARIES})

find_package(PNG REQUIRED)
include_directories(${PNG_INCLUDE_DIR})
target_link_libraries(rawproc ${PNG_LIBRARIES})
target_link_libraries(img ${PNG_LIBRARIES})
target_link_libraries(exif ${PNG_LIBRARIES})


#LittleCMS2:
#Github repo, autotools configure/make, retrive from build location references
if (BUILD_LCMS2)
	message(STATUS "Configuring lcms2 2.16 as an external project.")
	include(ExternalProject)
	ExternalProject_Add(lcms2_download
		PREFIX external/lcms2
		GIT_REPOSITORY    https://github.com/mm2/Little-CMS.git
		GIT_TAG           453baf #lcms2.16
		GIT_PROGRESS      ON
		CONFIGURE_COMMAND ../lcms2_download/configure --enable-static --disable-shared --with-threads --with-fastfloat
		INSTALL_COMMAND  ""   #make install DESTDIR=${CMAKE_CURRENT_BINARY_DIR}/external
		
	)
	
	set(LCMS2_INCLUDE_DIR ${CMAKE_CURRENT_BINARY_DIR}/external/lcms2/src/lcms2_download/include)
	set(LCMS2_LIB_DIR ${CMAKE_CURRENT_BINARY_DIR}/external/lcms2/src/lcms2_download-build/src/.libs)
	set(LCMS2_FASTFLOAT_INCLUDE_DIR ${CMAKE_CURRENT_BINARY_DIR}/external/lcms2/src/lcms2_download/plugins/fast_float/include)
	set(LCMS2_LIB_FASTFLOAT_DIR ${CMAKE_CURRENT_BINARY_DIR}/external/lcms2/src/lcms2_download-build/plugins/fast_float/src/.libs)
	
	add_library(lcms2 STATIC IMPORTED)
	add_library(lcms2_fastfloat STATIC IMPORTED)
	set_target_properties(lcms2 PROPERTIES IMPORTED_LOCATION ${LCMS2_LIB_DIR}/liblcms2.a)
	set_target_properties(lcms2_fastfloat PROPERTIES IMPORTED_LOCATION ${LCMS2_LIB_FASTFLOAT_DIR}/liblcms2_fast_float.a)
	set(LCMS2_FOUND TRUE)
	add_dependencies(rawproc lcms2_download)
	
	include_directories(${LCMS2_INCLUDE_DIR} ${LCMS2_FASTFLOAT_INCLUDE_DIR})
	target_link_libraries(rawproc lcms2 lcms2_fastfloat)
	target_link_libraries(img lcms2 lcms2_fastfloat)
	target_link_libraries(exif lcms2 lcms2_fastfloat)
	
	target_compile_options(rawproc PRIVATE -DUSE_LCMS_FASTFLOAT)
	target_compile_options(img PRIVATE -DUSE_LCMS_FASTFLOAT)
	target_compile_options(exif PRIVATE -DUSE_LCMS_FASTFLOAT)
else()
	pkg_check_modules(LCMS2 REQUIRED lcms2)
	if (LCMS2_FOUND)
		include_directories(${LCMS2_INCLUDE_DIR})
		target_link_libraries(rawproc ${LCMS2_LIBRARIES})
		target_link_libraries(img ${LCMS2_LIBRARIES})
		target_link_libraries(exif ${LCMS2_LIBRARIES})
		#if (LCMS2_FASTFLOAT)
			target_compile_options(rawproc PRIVATE -DUSE_LCMS_FASTFLOAT)
			target_compile_options(img PRIVATE -DUSE_LCMS_FASTFLOAT)
			target_compile_options(exif PRIVATE -DUSE_LCMS_FASTFLOAT)
			message(STATUS "LittleCMS2 FAST FLOAT enabled...")
		#endif (LCMS2_FASTFLOAT)
	else()
		message(STATUS "lcms2 not found")
	endif(LCMS2_FOUND)
endif(BUILD_LCMS2)

#Libraw:
#URL download, autotools configure/make, no install, retrive from build location references
if (BUILD_LIBRAW)
	message(STATUS "Configuring libraw 0.21.1 as an external project.")
	include(ExternalProject)
	ExternalProject_Add(libraw_download
		PREFIX external/libraw
		URL	https://www.libraw.org/data/LibRaw-0.21.2.tar.gz
		#URL	https://www.libraw.org/data/LibRaw-0.20.2.tar.gz
		DOWNLOAD_NO_PROGRESS true
		CONFIGURE_COMMAND ../libraw_download/configure --enable-static --disable-shared --enable-openmp --disable-jasper --disable-jpeg --disable-lcms --disable-examples
		INSTALL_COMMAND make install DESTDIR=${CMAKE_CURRENT_BINARY_DIR}/external
		
	)
	#set(LIBRAW_INCLUDE_DIR ${CMAKE_CURRENT_BINARY_DIR}/external/libraw/src/libraw_download/libraw)
	#set(LIBRAW_LIB_DIR ${CMAKE_CURRENT_BINARY_DIR}/external/libraw/src/libraw_download-build/lib/.libs)
	
	set(LIBRAW_INCLUDE_DIR ${CMAKE_CURRENT_BINARY_DIR}/external/usr/local/include)
	set(LIBRAW_LIB_DIR ${CMAKE_CURRENT_BINARY_DIR}/external/usr/local/lib)
	
	add_library(libraw STATIC IMPORTED)
	set_target_properties(libraw PROPERTIES IMPORTED_LOCATION ${LIBRAW_LIB_DIR}/libraw.a)
	set(LIBRAW_FOUND TRUE)
	add_dependencies(rawproc libraw_download)
	
	include_directories(${LIBRAW_INCLUDE_DIR})
	target_link_libraries(rawproc libraw -lz -lm)
	target_link_libraries(img libraw -lz -lm)
	target_link_libraries(exif libraw -lz -lm)
	
else()
	pkg_check_modules(LIBRAW REQUIRED libraw)
	if (LIBRAW_FOUND)
		include_directories(${LIBRAW_INCLUDE_DIR})
		target_link_libraries(rawproc ${LIBRAW_LIBRARIES})
		target_link_libraries(img ${LIBRAW_LIBRARIES})
		target_link_libraries(exif ${LIBRAW_LIBRARIES})
	else()
		message(STATUS "libraw not found")
	endif(LIBRAW_FOUND)
endif(BUILD_LIBRAW)


#Lensfun:
#Github repo, cmake, right now install to in-tree location, will change to build location references
if (BUILD_LENSFUN)
	message(STATUS "Configuring lensfun 0.3.99 (last code commit) as an external project.")
	include(ExternalProject)
	ExternalProject_Add(lensfun_download
		PREFIX external/lensfun
		GIT_REPOSITORY    https://github.com/lensfun/lensfun.git
		GIT_TAG           35c0017
		GIT_PROGRESS      ON
		CMAKE_ARGS -DCMAKE_BUILD_TYPE=RELEASE -DBUILD_STATIC=ON -DBUILD_TESTS=OFF -DBUILD_LENSTOOL=OFF -DINSTALL_PYTHON_MODULE=OFF -DINSTALL_HELPER_SCRIPTS=OFF -DCMAKE_INSTALL_PREFIX:PATH=external
		#INSTALL_COMMAND ""
	)
	set(LENSFUN_INCLUDE_DIR ${CMAKE_CURRENT_BINARY_DIR}/external//lensfun/src/lensfun_download-build/external/include)
	set(LENSFUN_LIB_DIR ${CMAKE_CURRENT_BINARY_DIR}/external/lensfun/src/lensfun_download-build//external/lib)

	
	add_library(lensfun STATIC IMPORTED)
	set_target_properties(lensfun PROPERTIES IMPORTED_LOCATION ${LENSFUN_LIB_DIR}/liblensfun.a)
	set(LENSFUN_FOUND TRUE)
	add_dependencies(rawproc lensfun_download)
	
	#include_directories(${LENSFUN_INCLUDE_DIR})
	target_include_directories(rawproc PRIVATE ${LENSFUN_INCLUDE_DIR})
	target_include_directories(img PRIVATE ${LENSFUN_INCLUDE_DIR})
	target_include_directories(exif PRIVATE ${LENSFUN_INCLUDE_DIR})
	target_link_libraries(rawproc lensfun -lglib-2.0)
	target_link_libraries(img lensfun -lglib-2.0)
	target_link_libraries(exif lensfun -lglib-2.0)
else()
	pkg_check_modules(LENSFUN REQUIRED lensfun)
	if (LENSFUN_FOUND)
		include_directories(${LENSFUN_INCLUDE_DIR})
		target_link_libraries(rawproc ${LENSFUN_LIBRARIES})
		target_link_libraries(img ${LENSFUN_LIBRARIES})
		target_link_libraries(exif ${LENSFUN_LIBRARIES})
	else()
		message(STATUS "lensfun not found")
	endif(LENSFUN_FOUND)
endif(BUILD_LENSFUN)



#exiv2:
#Github repo, cmake, install in-tree to resolve .in header creation
if (BUILD_EXIV2)
	message(STATUS "Configuring exiv2 0.27.1 as an external project.")
	include(ExternalProject)
	ExternalProject_Add(exiv2_download
		PREFIX external/exiv2
		GIT_REPOSITORY    https://github.com/Exiv2/exiv2.git
		GIT_TAG           3c648b #v0.27.7
		GIT_PROGRESS      ON
		CMAKE_ARGS   -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=OFF -DEXIV2_ENABLE_XMP=OFF -DEXIV2_BUILD_EXIV2_COMMAND=OFF -DEXIV2_BUILD_SAMPLES=OFF -DCMAKE_INSTALL_PREFIX:PATH=external
	)
	set(EXIV2_INCLUDE_DIR ${CMAKE_CURRENT_BINARY_DIR}/external/exiv2/src/exiv2_download-build/external/include)
	set(EXIV2_LIB_DIR ${CMAKE_CURRENT_BINARY_DIR}/external/exiv2/src/exiv2_download-build/external/lib)
	
	add_library(exiv2 STATIC IMPORTED)
	set_target_properties(exiv2 PROPERTIES IMPORTED_LOCATION ${EXIV2_LIB_DIR}/libexiv2.a)
	set(EXIV2_FOUND TRUE)
	add_dependencies(rawproc exiv2_download)
	
	include_directories(${EXIV2_INCLUDE_DIR})
	target_link_libraries(rawproc exiv2)
	target_link_libraries(img exiv2)
	target_link_libraries(exif exiv2)
	
	#for 0.27, take out if code upgraded to 0.28
	target_compile_options(rawproc PRIVATE -Wno-deprecated-declarations)
	target_compile_options(img PRIVATE -Wno-deprecated-declarations)
	target_compile_options(exif PRIVATE -Wno-deprecated-declarations)
	
	
else()
	pkg_check_modules(EXIV2 REQUIRED exiv2)
	if (EXIV2_FOUND)
		include_directories(${EXIV2_INCLUDE_DIR})
		target_link_libraries(rawproc ${EXIV2_LIBRARIES})
		target_link_libraries(img ${EXIV2_LIBRARIES})
		target_link_libraries(exif ${EXIV2_LIBRARIES})
	else()
		message(STATUS "exiv2 not found")
	endif(EXIV2_FOUND)
endif (BUILD_EXIV2)


#Optional Features:

#gmic:
if (GMICINC AND GMICLIB)
	include_directories("${GMICINC}")
	target_link_libraries(rawproc "${GMICLIB}")
	target_link_libraries(img "${GMICLIB}")
	target_link_libraries(exif "${GMICLIB}")
	target_compile_options(rawproc PRIVATE -DUSE_GMIC)
	target_compile_options(img PRIVATE -DUSE_GMIC)
	target_compile_options(exif PRIVATE -DUSE_GMIC)
	if (GMICFLAGS)
		message(STATUS "GMIC hand-configured with GMICFLAGS, GMICINC and GMICLIB")
	else()
		message(STATUS "GMIC hand-configured with GMICINC and GMICLIB")
	endif (GMICFLAGS)
elseif(GMIC)
	pkg_check_modules(GMIC REQUIRED gmic)
	if (GMIC_FOUND)
		include_directories(${GMIC_INCLUDE_DIR})
		target_link_libraries(rawproc ${GMIC_LIBRARIES})
		target_link_libraries(img ${GMIC_LIBRARIES})
		target_link_libraries(exif ${GMIC_LIBRARIES})
		target_compile_options(rawproc PRIVATE -DUSE_GMIC)
		target_compile_options(img PRIVATE -DUSE_GMIC)
		target_compile_options(exif PRIVATE -DUSE_GMIC)
	else()
		message(STATUS "gmic not found")
	endif(GMIC_FOUND)
endif (GMICINC AND GMICLIB)
	
#librtprocess
#Github repo, cmake, 
if (BUILD_LIBRTPROCESS)
	message(STATUS "Configuring librtprocess as an external project.")
	include(ExternalProject)
	ExternalProject_Add(librtprocess_download
		PREFIX external/librtprocess
		GIT_REPOSITORY   https://github.com/CarVac/librtprocess.git
		GIT_TAG          ed4501 #0.12.0
		GIT_PROGRESS     ON
		CMAKE_ARGS   -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=OFF -DCMAKE_INSTALL_PREFIX:PATH=external
		INSTALL_COMMAND ""
	)
	set(LIBRTPROCESS_INCLUDE_DIR ${CMAKE_CURRENT_BINARY_DIR}/external/librtprocess/src/librtprocess_download/src/include)
	set(LIBRTPROCESS_LIB_DIR ${CMAKE_CURRENT_BINARY_DIR}/external/librtprocess/src/librtprocess_download-build/src)
	
	add_library(librtprocess STATIC IMPORTED)
	set_target_properties(librtprocess PROPERTIES IMPORTED_LOCATION ${LIBRTPROCESS_LIB_DIR}/librtprocess.a)
	set(LIBRTPROCESS_FOUND TRUE)
	add_dependencies(rawproc librtprocess_download)
	
	include_directories(${LIBRTPROCESS_INCLUDE_DIR})
	target_link_libraries(rawproc librtprocess)
	target_link_libraries(img librtprocess)
	target_link_libraries(exif librtprocess)
	
	target_compile_options(rawproc PRIVATE -DUSE_LIBRTPROCESS)
	target_compile_options(img PRIVATE -DUSE_LIBRTPROCESS)
	target_compile_options(exif PRIVATE -DUSE_LIBRTPROCESS)
else()
	pkg_check_modules(LIBRTPROCESS REQUIRED rtprocess)
	if (LIBRTPROCESS_FOUND)
		target_compile_options(rawproc PRIVATE -DUSE_LIBRTPROCESS)
		target_compile_options(img PRIVATE -DUSE_LIBRTPROCESS)
		target_compile_options(exif PRIVATE -DUSE_LIBRTPROCESS)
		include_directories(${LIBRTPROCESS_INCLUDE_DIR})
		target_link_libraries(rawproc ${LIBRTPROCESS_LIBRARIES})
		target_link_libraries(img ${LIBRTPROCESS_LIBRARIES})
		target_link_libraries(exif ${LIBRTPROCESS_LIBRARIES})
	else()
		message(STATUS "librtprocess not found")
	endif(LIBRTPROCESS_FOUND)
endif (BUILD_LIBRTPROCESS)


#lensfun dbupdate, require libcurl and libarchive:
if(LENSFUN_DBUPDATE)
	find_package(CURL REQUIRED)
	include_directories(${CURL_INCLUDE_DIR})
	target_link_libraries(rawproc ${CURL_LIBRARIES})
	
	find_package(LibArchive REQUIRED)
	include_directories(${LibArchive_INCLUDE_DIR})
	target_link_libraries(rawproc ${LibArchive_LIBRARIES})
	
	target_compile_options(rawproc PRIVATE -DUSE_LENSFUN_DBUPDATE)
	
endif(LENSFUN_DBUPDATE)

#Just put it in at the end...
target_link_libraries(rawproc -lz)
target_link_libraries(img -lz)
target_link_libraries(exif -lz)

if(WIN32)
	target_link_libraries(rawproc wsock32 ws2_32)
	target_link_libraries(img wsock32 ws2_32)
	target_link_libraries(exif wsock32 ws2_32)
endif(WIN32)


