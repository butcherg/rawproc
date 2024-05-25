
#cmake_minimum_required(VERSION 3.15)
cmake_minimum_required(VERSION 3.22)
cmake_policy(SET CMP0017 NEW)
cmake_policy(SET CMP0020 NEW)

#include(cmake/UniversalAppleBuild.cmake)

MESSAGE("CMAKE_HOST_SYSTEM_NAME ${CMAKE_HOST_SYSTEM_NAME}")
MESSAGE("CMAKE_HOST_SYSTEM_VERSION ${CMAKE_HOST_SYSTEM_VERSION}")
project(rawproc VERSION 1.4.0 LANGUAGES CXX C)

#set(VERSION 1.4)
string(TIMESTAMP BUILDDATE)

add_compile_options(-ggdb -DVERSION="${CMAKE_PROJECT_VERSION}"  -DBUILDDATE="${BUILDDATE}")

add_executable(rawproc src/rawprocApp.cpp ) #$<TARGET_OBJECTS:gimage> $<TARGET_OBJECTS:gimage_proc> $<TARGET_OBJECTS:util>)
add_executable(img src/img.cpp ) #$<TARGET_OBJECTS:gimage> $<TARGET_OBJECTS:gimage_proc> $<TARGET_OBJECTS:util>)
add_executable(wxcmd src/wxcmd.cpp)
add_executable(exif src/exif.cpp ) #$<TARGET_OBJECTS:gimage> $<TARGET_OBJECTS:util>)

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

add_custom_target(conf
	COMMAND rm -f ${CMAKE_CURRENT_BINARY_DIR}/rawproc.conf ${CMAKE_CURRENT_BINARY_DIR}/dcraw.c ${CMAKE_CURRENT_BINARY_DIR}/camconst.json
	COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/conffile.pl ${CMAKE_CURRENT_SOURCE_DIR}/src > ${CMAKE_CURRENT_BINARY_DIR}/rawproc.conf
	COMMAND wget -P ${CMAKE_CURRENT_BINARY_DIR} https://raw.githubusercontent.com/Beep6581/RawTherapee/dev/rtengine/camconst.json 
	COMMAND wget -P ${CMAKE_CURRENT_BINARY_DIR}. https://raw.githubusercontent.com/Beep6581/RawTherapee/dev/rtengine/dcraw.c
	#Not using the following anymore, easier to rely on rawproc's lensfun update capability
	#COMMAND wget -P ${CMAKE_CURRENT_BINARY_DIR} "http://lensfun.sourceforge.net/db/version_${LENSFUN_DATABASE_VERSION}.tar.bz2"
	#COMMAND mkdir -p "${CMAKE_CURRENT_BINARY_DIR}/version_toohardtofigureout"
	#COMMAND tar -xjvf ${CMAKE_CURRENT_BINARY_DIR}/${LENSFUN_DATABASE_VERSION}.tar.bz2 -C ${LENSFUN_DATABASE_VERSION}
	#COMMAND echo "0" >${CMAKE_CURRENT_BINARY_DIR}/ ${LENSFUN_DATABASE_VERSION}/timestamp.txt
	VERBATIM
)

add_custom_target(docpages
	COMMAND sed -e s/\#VERSION\#/${CMAKE_PROJECT_VERSION}/ -e s/\#DATE\#/${BUILDDATE}/ ${CMAKE_CURRENT_SOURCE_DIR}/doc/main.html.in > ${CMAKE_CURRENT_SOURCE_DIR}/doc/main.html
	COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/doc/confparams.pl ${CMAKE_CURRENT_SOURCE_DIR}/src > ${CMAKE_CURRENT_SOURCE_DIR}/doc/confparams.html
	COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/doc/rawparams.pl ${CMAKE_CURRENT_SOURCE_DIR}/src/rawimage.cpp > ${CMAKE_CURRENT_SOURCE_DIR}/doc/rawparams.html
	COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/doc/img.pl ${CMAKE_CURRENT_SOURCE_DIR}/src/gimage_parse.cpp > ${CMAKE_CURRENT_SOURCE_DIR}/doc/img.html
	COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/doc/index.pl ${CMAKE_CURRENT_SOURCE_DIR}/doc > ${CMAKE_CURRENT_SOURCE_DIR}/doc/index.html
	VERBATIM
)


#add_subdirectory(src)

find_package(OpenMP)
if(OpenMP_CXX_FOUND)
	target_link_libraries(rawproc  OpenMP::OpenMP_CXX)
	target_link_libraries(img  OpenMP::OpenMP_CXX)
	target_link_libraries(exif  OpenMP::OpenMP_CXX)
endif()

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

#WxWidgets:
#if WXDIR contains a path to a built wxWidgets library set, wx-config there will be used to provide the includes and libs.  Otherwise, a system package will be found.
if(DEFINED WXDIR)
	execute_process(COMMAND bash -c "${WXDIR}/wx-config --cxxflags" OUTPUT_VARIABLE WX_FLAGS OUTPUT_STRIP_TRAILING_WHITESPACE)
	execute_process(COMMAND bash -c "${WXDIR}/wx-config --libs base core propgrid aui html" OUTPUT_VARIABLE WX_LIBS OUTPUT_STRIP_TRAILING_WHITESPACE )
	string(APPEND CMAKE_CXX_FLAGS " ${WX_FLAGS}")
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
endif(DEFINED WXDIR)


#Establishes the target install directory for any built libraries:
set(BUILD_PREFIX "${CMAKE_CURRENT_BINARY_DIR}/external/usr")


#LittleCMS2:
#Github repo, autotools configure/make, retrive from build location references
if (DEFINED BUILD_LCMS2)
	if (DEFINED HOST)
		set(HOSTPARM "--host=${HOST}")
	else()
		set(HOSTPARM "")
	endif()
	include(ExternalProject)
	if (BUILD_LCMS2 STREQUAL "GITHUB")
		message(STATUS "Configuring lcms2 as an external project (GitHub clone).")
		ExternalProject_Add(lcms2_download
			PREFIX external/lcms2
			GIT_REPOSITORY    https://github.com/mm2/Little-CMS.git
			#GIT_TAG           453baf #lcms2.16
			GIT_PROGRESS      ON
			CONFIGURE_COMMAND ../lcms2_download/configure ${HOSTPARM} --enable-static --disable-shared --with-threads --with-fastfloat --prefix=${BUILD_PREFIX}
			INSTALL_COMMAND make install 
			UPDATE_COMMAND ""
		)
	elseif (BUILD_LCMS2 STREQUAL "SRCPKG")
		message(STATUS "Configuring lcms2 as an external project (URL download).")
		ExternalProject_Add(lcms2_download
			PREFIX external/lcms2
			#DOWNLOAD_DIR "external"
			URL https://github.com/mm2/Little-CMS/releases/download/lcms2.16/lcms2-2.16.tar.gz
			CONFIGURE_COMMAND ../lcms2_download/configure ${HOSTPARM} --enable-static --disable-shared --with-threads --with-fastfloat --prefix=${BUILD_PREFIX}
			INSTALL_COMMAND make install 
			UPDATE_COMMAND ""
			#DOWNLOAD_EXTRACT_TIMESTAMP true
		)
	elseif (EXISTS ${BUILD_LCMS2})  #local path to file
		message(STATUS "Configuring lcms2 as an external project (local file: ${BUILD_LCMS2})")
		ExternalProject_Add(lcms2_download
			PREFIX external/lcms2
			#DOWNLOAD_DIR "external"
			URL "${BUILD_LCMS2}" 
			CONFIGURE_COMMAND ../lcms2_download/configure ${HOSTPARM} --enable-static --disable-shared --with-threads --with-fastfloat --prefix=${BUILD_PREFIX}
			INSTALL_COMMAND make install 
			UPDATE_COMMAND ""
			#DOWNLOAD_EXTRACT_TIMESTAMP true
		)
	else ()
		message(FATAL_ERROR "BUILD_LCMS2 not valid: ${BUILD_LCMS2} (Either GITHUB, SRCPKG, or valid local path/srcpackage")
	endif(BUILD_LCMS2 STREQUAL "GITHUB")

	set(LCMS2_INCLUDE_DIR "${CMAKE_CURRENT_BINARY_DIR}/external/usr/include")
	set(LCMS2_LIB_DIR "${CMAKE_CURRENT_BINARY_DIR}/external/usr/lib")
	set(LCMS2_LIB "${CMAKE_CURRENT_BINARY_DIR}/external/usr/lib/liblcms2.a")
	set(LCMS2_FASTFLOAT_INCLUDE_DIR "${CMAKE_CURRENT_BINARY_DIR}/external/usr/include")
	set(LCMS2_LIB_FASTFLOAT_DIR "${CMAKE_CURRENT_BINARY_DIR}/external/usr/lib")
	set(LCMS2_LIB_FASTFLOAT "${CMAKE_CURRENT_BINARY_DIR}/external/usr/lib/liblcms2_fast_float.a")
	
	add_library(lcms2 STATIC IMPORTED)
	add_library(lcms2_fastfloat STATIC IMPORTED)
	set_target_properties(lcms2 PROPERTIES IMPORTED_LOCATION ${LCMS2_LIB_DIR}/liblcms2.a)
	set_target_properties(lcms2_fastfloat PROPERTIES IMPORTED_LOCATION ${LCMS2_LIB_FASTFLOAT_DIR}/liblcms2_fast_float.a)
	set(LCMS2_FOUND TRUE)
	add_dependencies(rawproc lcms2_download)
	
	target_include_directories(rawproc PUBLIC "${LCMS2_INCLUDE_DIR}" "${LCMS2_FASTFLOAT_INCLUDE_DIR}")
	target_include_directories(img PUBLIC "${LCMS2_INCLUDE_DIR}" "${LCMS2_FASTFLOAT_INCLUDE_DIR}" )
	target_include_directories(exif PUBLIC "${LCMS2_INCLUDE_DIR}" "${LCMS2_FASTFLOAT_INCLUDE_DIR}")

	target_link_libraries(rawproc "${LCMS2_LIB}" "${LCMS2_LIB_FASTFLOAT}" ${LCMS2_DEPS})
	target_link_libraries(img "${LCMS2_LIB}" "${LCMS2_LIB_FASTFLOAT}" ${LCMS2_DEPS})
	target_link_libraries(exif "${LCMS2_LIB}" "${LCMS2_LIB_FASTFLOAT}" ${LCMS2_DEPS})
	
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
		if (LCMS2_FASTFLOAT)
			target_compile_options(rawproc PRIVATE -DUSE_LCMS_FASTFLOAT)
			target_compile_options(img PRIVATE -DUSE_LCMS_FASTFLOAT)
			target_compile_options(exif PRIVATE -DUSE_LCMS_FASTFLOAT)
			message(STATUS "LittleCMS2 FAST FLOAT enabled...")
		endif (LCMS2_FASTFLOAT)
	else()
		message(STATUS "lcms2 not found")
	endif(LCMS2_FOUND)
endif ()

#Libraw:
#URL download, autotools configure/make, no install, retrive from build location references
if (DEFINED BUILD_LIBRAW)
	if (DEFINED HOST)
		set(HOSTPARM "--host=${HOST}")
	else()
		set(HOSTPARM "")
	endif()

	include(ExternalProject)
	if (BUILD_LIBRAW STREQUAL "GITHUB")
		message(STATUS "Configuring libraw as an external project (GitHub clone).")
		ExternalProject_Add(libraw_download
			PREFIX external/libraw
			GIT_REPOSITORY    https://github.com/LibRaw/LibRaw.git
			GIT_PROGRESS      ON
			CONFIGURE_COMMAND autoreconf -i <SOURCE_DIR> COMMAND ../libraw_download/configure ${HOSTPARM} --enable-static --disable-shared --enable-openmp --disable-jasper --disable-jpeg --disable-lcms --disable-examples --prefix=${BUILD_PREFIX}
			INSTALL_COMMAND make install
			UPDATE_COMMAND ""
		)
	elseif (BUILD_LIBRAW STREQUAL "SRCPKG")
		message(STATUS "Configuring libraw as an external project (URL download).")
		ExternalProject_Add(libraw_download
			PREFIX external/libraw
			#DOWNLOAD_DIR "external"
			URL	https://www.libraw.org/data/LibRaw-0.21.2.tar.gz
			CONFIGURE_COMMAND ../libraw_download/configure ${HOSTPARM} --enable-static --disable-shared --enable-openmp --disable-jasper --disable-jpeg --disable-lcms --disable-examples --prefix=${BUILD_PREFIX}
			INSTALL_COMMAND make install
			UPDATE_COMMAND ""
			#DOWNLOAD_EXTRACT_TIMESTAMP true
		)
	elseif (EXISTS ${BUILD_LIBRAW} )
		message(STATUS "Configuring libraw as an external project (local file: ${BUILD_LIBRAW}).")
		ExternalProject_Add(libraw_download
			PREFIX external/libraw 
			#DOWNLOAD_DIR "external"
			URL	 "${BUILD_LIBRAW}" 
			CONFIGURE_COMMAND ../libraw_download/configure ${HOSTPARM} --enable-static --disable-shared --enable-openmp --disable-jasper --disable-jpeg --disable-lcms --disable-examples --prefix=${BUILD_PREFIX} 
			INSTALL_COMMAND make install 
			UPDATE_COMMAND "" 
			#DOWNLOAD_EXTRACT_TIMESTAMP true 
		)
	else ()
		message(FATAL_ERROR "BUILD_LIBRAW not valid: ${BUILD_LIBRAW} (Either GITHUB, SRCPKG, or valid local path/srcpackage")
	endif(BUILD_LIBRAW STREQUAL "GITHUB")
	
	set(LIBRAW_INCLUDE_DIR ${CMAKE_CURRENT_BINARY_DIR}/external/usr/include)
	set(LIBRAW_LIB_DIR ${CMAKE_CURRENT_BINARY_DIR}/external/usr/lib)
	set(LIBRAW_LIB ${CMAKE_CURRENT_BINARY_DIR}/external/usr/lib/libraw.a)
	
	add_library(libraw STATIC IMPORTED)
	set_target_properties(libraw PROPERTIES IMPORTED_LOCATION ${LIBRAW_LIB_DIR}/libraw.a)
	set(LIBRAW_FOUND TRUE)
	add_dependencies(rawproc libraw_download)

	target_include_directories(rawproc PRIVATE ${LIBRAW_INCLUDE_DIR})
	target_include_directories(img PRIVATE ${LIBRAW_INCLUDE_DIR})
	target_include_directories(exif PRIVATE ${LIBRAW_INCLUDE_DIR})
	target_link_libraries(rawproc ${LIBRAW_LIB} ${LIBRAW_DEPS})
	target_link_libraries(img ${LIBRAW_LIB}  ${LIBRAW_DEPS})
	target_link_libraries(exif ${LIBRAW_LIB}  ${LIBRAW_DEPS})
	
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
endif ()

#Lensfun:
#Github repo, cmake, right now install to in-tree location, will change to build location references
#set(LENSFUN_DEPS "-lglib-2.0 -lintl -liconv -lpcre")
if (DEFINED BUILD_LENSFUN)
	if (DEFINED HOST)
		set(CMAKECMD "${HOST}-cmake")
	else()
		set(CMAKECMD "cmake")
	endif()
	set(CMAKE_ARGUMENTS -DCMAKE_BUILD_TYPE=RELEASE -DBUILD_STATIC=ON -DBUILD_TESTS=OFF -DBUILD_LENSTOOL=OFF -DINSTALL_PYTHON_MODULE=OFF -DINSTALL_HELPER_SCRIPTS=OFF -DCMAKE_INSTALL_PREFIX=${BUILD_PREFIX} -DCMAKE_INSTALL_DATAROOTDIR=share)

	include(ExternalProject)
	if (BUILD_LENSFUN STREQUAL "GITHUB")
		message(STATUS "Configuring lensfun 0.3.99 as an external project (GitHub clone).")
		ExternalProject_Add(lensfun_download
			PREFIX external/lensfun
			GIT_REPOSITORY    https://github.com/lensfun/lensfun.git
			GIT_PROGRESS      ON
			CMAKE_COMMAND	${CMAKECMD}
			CMAKE_ARGS ${CMAKE_ARGUMENTS}
			INSTALL_COMMAND make install
			UPDATE_COMMAND ""
		)
	elseif (BUILD_LENSFUN STREQUAL "SRCPKG")
		message(STATUS "Configuring lensfun 0.3.99 as an external project (URL download).")
		ExternalProject_Add(lensfun_download
			PREFIX external/lensfun
			#DOWNLOAD_DIR "external"
			URL https://github.com/lensfun/lensfun/archive/refs/tags/v0.3.4.tar.gz
			CMAKE_COMMAND	${CMAKECMD}
			CMAKE_ARGS ${CMAKE_ARGUMENTS}
			INSTALL_COMMAND make install
			UPDATE_COMMAND DOWNLOAD_EXTRACT_TIMESTAMP true
			#DOWNLOAD_EXTRACT_TIMESTAMP true
		)
	elseif (EXISTS ${BUILD_LENSFUN})
		message(STATUS "Configuring lensfun 0.3.99 as an external project (local file: ${BUILD_LENSFUN}).")
		ExternalProject_Add(lensfun_download
			URL ${BUILD_LENSFUN}
			PREFIX external/lensfun
			CMAKE_COMMAND	${CMAKECMD}
			CMAKE_ARGS ${CMAKE_ARGUMENTS}
			INSTALL_COMMAND make install
			UPDATE_COMMAND ""
		)
	else ()
		message(FATAL_ERROR "BUILD_LENSFUN not valid: ${BUILD_LENSFUN} (Either GITHUB, SRCPKG, or valid local path/srcpackage")
	endif()

	set(LENSFUN_INCLUDE_DIR ${CMAKE_CURRENT_BINARY_DIR}/external/usr/include)
	set(LENSFUN_INC_DIR ${LENSFUN_INCLUDE_DIR} CACHE STRING "Lensfun include directory")
	set(LENSFUN_LIB_DIR ${CMAKE_CURRENT_BINARY_DIR}/external/usr/lib)
	set(LENSFUN_LIB ${CMAKE_CURRENT_BINARY_DIR}/external/usr/lib/liblensfun.a)
	
	add_library(lensfun STATIC IMPORTED)
	set_target_properties(lensfun PROPERTIES IMPORTED_LOCATION ${LENSFUN_LIB_DIR}/liblensfun.a)
	set(LENSFUN_FOUND TRUE)
	add_dependencies(rawproc lensfun_download)
	
	target_include_directories(rawproc PRIVATE ${LENSFUN_INCLUDE_DIR})
	target_include_directories(img PRIVATE ${LENSFUN_INCLUDE_DIR})
	target_include_directories(exif PRIVATE ${LENSFUN_INCLUDE_DIR})
	target_link_libraries(rawproc ${LENSFUN_LIB}  ${LENSFUN_DEPS})
	target_link_libraries(img ${LENSFUN_LIB} ${LENSFUN_DEPS})
	target_link_libraries(exif ${LENSFUN_LIB} ${LENSFUN_DEPS})
else()
	pkg_check_modules(LENSFUN REQUIRED lensfun)
	if (LENSFUN_FOUND)
		set(LENSFUN_INC_DIR ${LENSFUN_INCLUDE_DIR} CACHE STRING "Lensfun include directory")
		include_directories(${LENSFUN_INCLUDE_DIR})
		target_link_libraries(rawproc ${LENSFUN_LIBRARIES})
		target_link_libraries(img ${LENSFUN_LIBRARIES})
		target_link_libraries(exif ${LENSFUN_LIBRARIES})
	else()
		message(STATUS "lensfun not found")
	endif(LENSFUN_FOUND)
endif ()


#exiv2:
#Github repo, cmake, install in-tree to resolve .in header creation
#set(EXIV2_DEPS "-lintl -liconv -lcrypto -lssl -lz")  # expat gettext mman-win32 zlib
#if (WIN32)
#	string(CONCAT EXIV2_DEPS "-lws2_32 -lpsapi")
#endif()
if  (DEFINED BUILD_EXIV2)
	if (DEFINED HOST)
		set(CMAKECMD "${HOST}-cmake")
	else()
		set(CMAKECMD "cmake")
	endif()
	set(CMAKE_ARGUMENTS -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=OFF -DEXIV2_ENABLE_XMP=OFF -DEXIV2_ENABLE_PNG=OFF -DEXIV2_ENABLE_NLS=OFF -DEXIV2_ENABLE_PRINTUCS2=OFF -DEXIV2_BUILD_SAMPLES=OFF -DEXIV2_BUILD_EXIV2_COMMAND=OFF -DCMAKE_INSTALL_PREFIX:PATH=${BUILD_PREFIX})
	
	include(ExternalProject)
	if (BUILD_EXIV2 STREQUAL "GITHUB")
		message(STATUS "Configuring exiv2 0.27.7 as an external project (GitHub clone).")
		ExternalProject_Add(exiv2_download
			PREFIX external/exiv2
			GIT_REPOSITORY    https://github.com/Exiv2/exiv2.git
			GIT_TAG           3c648b #v0.27.7, rawproc can't do 0.28 yet...
			GIT_PROGRESS      ON
			CMAKE_COMMAND	${CMAKECMD}
			CMAKE_ARGS   ${CMAKE_ARGUMENTS}
			INSTALL_COMMAND make install
			UPDATE_COMMAND ""
		)
	elseif (BUILD_EXIV2 STREQUAL "SRCPKG")
		message(STATUS "Configuring exiv2 0.27.7 as an external project (URL download).")
		ExternalProject_Add(exiv2_download
			PREFIX external/exiv2
			#DOWNLOAD_DIR "external"
			URL https://github.com/Exiv2/exiv2/releases/download/v0.27.7/exiv2-0.27.7-Source.tar.gz
			CMAKE_COMMAND	${CMAKECMD}
			CMAKE_ARGS   ${CMAKE_ARGUMENTS}
			INSTALL_COMMAND make install
			UPDATE_COMMAND ""
			#DOWNLOAD_EXTRACT_TIMESTAMP true
		)
	elseif (EXISTS ${BUILD_EXIV2})
		message(STATUS "Configuring exiv2 0.27.7 as an external project (local file: ${BUILD_EXIV2}).")
		ExternalProject_Add(exiv2_download
			PREFIX external/exiv2
			#DOWNLOAD_DIR "external"
			URL "${BUILD_EXIV2}"
			CMAKE_COMMAND	${CMAKECMD}
			CMAKE_ARGS    ${CMAKE_ARGUMENTS}
			INSTALL_COMMAND make install
			UPDATE_COMMAND ""
			#DOWNLOAD_EXTRACT_TIMESTAMP true
		)
	else ()
		message(FATAL_ERROR "BUILD_EXIV2 not valid: ${BUILD_EXIV2} (Either GITHUB, SRCPKG, or valid local path/srcpackage")
	endif()
	
	set(EXIV2_INCLUDE_DIR ${CMAKE_CURRENT_BINARY_DIR}/external/usr/include)
	set(EXIV2_LIB_DIR ${CMAKE_CURRENT_BINARY_DIR}/external/usr/lib)
	set(EXIV2_LIB ${CMAKE_CURRENT_BINARY_DIR}/external/usr/lib/libexiv2.a)
	
	add_library(exiv2 STATIC IMPORTED)
	set_target_properties(exiv2 PROPERTIES IMPORTED_LOCATION ${EXIV2_LIB_DIR}/libexiv2.a)
	set(EXIV2_FOUND TRUE)
	add_dependencies(rawproc exiv2_download)
	
	target_include_directories(rawproc PRIVATE ${EXIV2_INCLUDE_DIR})
	target_include_directories(img PRIVATE ${EXIV2_INCLUDE_DIR})
	target_include_directories(exif PRIVATE ${EXIV2_INCLUDE_DIR})
	target_link_libraries(rawproc ${EXIV2_LIB} ${EXIV2_DEPS})
	target_link_libraries(img  ${EXIV2_LIB} ${EXIV2_DEPS})
	target_link_libraries(exif  ${EXIV2_LIB} ${EXIV2_DEPS})
	
	#for 0.27, take out if code upgraded to 0.28
	target_compile_options(rawproc PRIVATE -Wno-deprecated-declarations)
	target_compile_options(img PRIVATE -Wno-deprecated-declarations)
	target_compile_options(exif PRIVATE -Wno-deprecated-declarations)
	
else()
	pkg_check_modules(EXIV2 REQUIRED exiv2)
	if (EXIV2_FOUND)
		include_directories(${EXIV2_INCLUDE_DIR})
		target_link_libraries(rawproc ${EXIV2_LIBRARIES} ${EXIV2_DEPS})
		target_link_libraries(img ${EXIV2_LIBRARIES}  ${EXIV2_DEPS})
		target_link_libraries(exif ${EXIV2_LIBRARIES}  ${EXIV2_DEPS})
	else()
		message(STATUS "exiv2 not found")
	endif(EXIV2_FOUND)
endif()


#Optional Features:

#gmic:
if (DEFINED BUILD_GMIC)
	if (DEFINED HOST)
		set(CMAKECMD "${HOST}-cmake")
	else()
		set(CMAKECMD "cmake")
	endif()
	set(CMAKE_ARGUMENTS -DCMAKE_CXX_FLAGS="-DCURL_STATICLIB" -DBUILD_LIB=OFF -DBUILD_LIB_STATIC=ON -DBUILD_CLI=OFF -DBUILD_MAN=OFF -DBUILD_BASH_COMPLETION=OFF -DCMAKE_INSTALL_PREFIX:PATH=${BUILD_PREFIX})
	include(ExternalProject)
	if (BUILD_GMIC STREQUAL "GITHUB")
		message(STATUS "Configuring G'MIC as an external project (GitHub clone).")
		ExternalProject_Add(gmic_download
			PREFIX external/gmic
			GIT_REPOSITORY   https://github.com/GreycLab/gmic.git
			GIT_PROGRESS     ON
			CONFIGURE_COMMAND ""
			BUILD_IN_SOURCE true
			BUILD_COMMAND make X11_CFLAGS="-Dcimg_display=0" X11_LIBS="" PREFIX=${BUILD_PREFIX} lib
			INSTALL_COMMAND cd src COMMAND make install
			UPDATE_COMMAND ""
		)
	elseif (BUILD_GMIC STREQUAL "SRCPKG")
		message(STATUS "Configuring G'MIC as an external project (URL download).")
		ExternalProject_Add(gmic_download
			PREFIX external/gmic
			#DOWNLOAD_EXTRACT_TIMESTAMP true
			URL https://github.com/GreycLab/gmic/releases/download/v.3.3.6/gmic_3.3.6.tar.gz
			CONFIGURE_COMMAND ""
			BUILD_IN_SOURCE true
			BUILD_COMMAND make X11_CFLAGS="-Dcimg_display=0" X11_LIBS="" PREFIX=${BUILD_PREFIX} lib
			INSTALL_COMMAND cd src COMMAND make install
			UPDATE_COMMAND ""
			#DOWNLOAD_EXTRACT_TIMESTAMP true
		)
	elseif (EXISTS ${BUILD_GMIC})
		message(STATUS "Configuring G'MIC as an external project (local file: ${BUILD_GMIC}).")
		ExternalProject_Add(gmic_download
			PREFIX external/gmic
			#DOWNLOAD_DIR "external"
			URL "${BUILD_GMIC}"
			CONFIGURE_COMMAND ""
			BUILD_IN_SOURCE true
			BUILD_COMMAND make X11_CFLAGS="-Dcimg_display=0" X11_LIBS="" PREFIX=${BUILD_PREFIX} lib
			INSTALL_COMMAND cd src COMMAND make install
			UPDATE_COMMAND ""
			#DOWNLOAD_EXTRACT_TIMESTAMP true
		)
	else ()
		message(FATAL_ERROR "BUILD_GMIC not valid: ${BUILD_GMIC} (Either GITHUB, SRCPKG, or valid local path/srcpackage")
	endif()
		
		
	set(GMIC_INCLUDE_DIR ${CMAKE_CURRENT_BINARY_DIR}/external/usr/include)
	set(GMIC_LIB_DIR ${CMAKE_CURRENT_BINARY_DIR}/external/usr/lib)
	set(GMIC_LIB ${CMAKE_CURRENT_BINARY_DIR}/external/usr/lib/libgmic.a)
	
	add_library(gmic STATIC IMPORTED)
	set_target_properties(gmic PROPERTIES IMPORTED_LOCATION ${GMIC_LIB_DIR}/libgmic.a)
	set(GMIC_FOUND TRUE)
	add_dependencies(gmic gmic_download)
	
	include_directories(${GMIC_INCLUDE_DIR})
	target_link_libraries(rawproc gmic ${GMIC_DEPS}) 
	target_link_libraries(img gmic ${GMIC_DEPS})
	target_link_libraries(exif gmic ${GMIC_DEPS})
	
	target_compile_options(rawproc PRIVATE -DUSE_GMIC)
	target_compile_options(img PRIVATE -DUSE_GMIC)
	target_compile_options(exif PRIVATE -DUSE_GMIC)
	
elseif(GMIC)
	pkg_check_modules(GMIC REQUIRED gmic)
	if (GMIC_FOUND)
		include_directories(${GMIC_INCLUDE_DIR})
		target_link_libraries(rawproc ${GMIC_LIBRARIES} ${GMIC_DEPS})
		target_link_libraries(img ${GMIC_LIBRARIES} ${GMIC_DEPS})
		target_link_libraries(exif ${GMIC_LIBRARIES} ${GMIC_DEPS})
		target_compile_options(rawproc PRIVATE -DUSE_GMIC)
		target_compile_options(img PRIVATE -DUSE_GMIC)
		target_compile_options(exif PRIVATE -DUSE_GMIC)
	else()
		message(STATUS "gmic not found")
	endif(GMIC_FOUND)
endif ()
	
#librtprocess
#Github repo, cmake, 
if (DEFINED BUILD_LIBRTPROCESS)
	if (DEFINED HOST)
		set(CMAKECMD "${HOST}-cmake")
	else()
		set(CMAKECMD "cmake")
	endif()
	set(CMAKE_ARGUMENTS -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=OFF -DCMAKE_INSTALL_PREFIX:PATH=${BUILD_PREFIX})
	include(ExternalProject)
	if (BUILD_LIBRTPROCESS STREQUAL "GITHUB")
		message(STATUS "Configuring librtprocess as an external project (GitHub clone).")
		ExternalProject_Add(librtprocess_download
			PREFIX external/librtprocess
			GIT_REPOSITORY   https://github.com/CarVac/librtprocess.git
			GIT_TAG          ed4501 #0.12.0
			GIT_PROGRESS     ON
			CMAKE_COMMAND	${CMAKECMD}
			CMAKE_ARGS  ${CMAKE_ARGUMENTS}
			INSTALL_COMMAND make install
			UPDATE_COMMAND ""
		)
	elseif (BUILD_LIBRTPROCESS STREQUAL "SRCPKG")
		message(STATUS "Configuring librtprocess as an external project (URL download).")
		ExternalProject_Add(librtprocess_download
			PREFIX external/librtprocess
			#DOWNLOAD_DIR "external"
			URL https://github.com/CarVac/librtprocess/archive/refs/tags/0.12.0.tar.gz
			CMAKE_COMMAND	${CMAKECMD}
			CMAKE_ARGS  ${CMAKE_ARGUMENTS}
			INSTALL_COMMAND make install
			UPDATE_COMMAND ""
			#DOWNLOAD_EXTRACT_TIMESTAMP true
		)
	elseif (EXISTS ${BUILD_LIBRTPROCESS})
		message(STATUS "Configuring librtprocess as an external project (local file: ${BUILD_LIBRTPROCESS}).")
		ExternalProject_Add(librtprocess_download
			PREFIX external/librtprocess
			#DOWNLOAD_DIR "external"
			URL "${BUILD_LIBRTPROCESS}"
			CMAKE_COMMAND	${CMAKECMD}
			CMAKE_ARGS  ${CMAKE_ARGUMENTS}
			INSTALL_COMMAND make install
			UPDATE_COMMAND ""
			#DOWNLOAD_EXTRACT_TIMESTAMP true
		)
	else ()
		message(FATAL_ERROR "BUILD_LIBRTPROCESS not valid: ${BUILD_LIBRTPROCESS} (Either GITHUB, SRCPKG, or valid local path/srcpackage")
	endif()
	
	set(LIBRTPROCESS_INCLUDE_DIR ${CMAKE_CURRENT_BINARY_DIR}/external/usr/include)
	set(LIBRTPROCESS_LIB_DIR ${CMAKE_CURRENT_BINARY_DIR}/external/usr/lib)
	set(LIBRTPROCESS_LIB ${CMAKE_CURRENT_BINARY_DIR}/external/usr/lib/librtprocess.a)
	
	add_library(librtprocess STATIC IMPORTED)
	set_target_properties(librtprocess PROPERTIES IMPORTED_LOCATION ${LIBRTPROCESS_LIB_DIR}/librtprocess.a)
	set(LIBRTPROCESS_FOUND TRUE)
	add_dependencies(rawproc librtprocess_download)
	
	include_directories(${LIBRTPROCESS_INCLUDE_DIR})
	target_link_libraries(rawproc librtprocess)
	target_link_libraries(img librtprocess)
	target_link_libraries(exif librtprocess)
	
	add_definitions(-DUSE_LIBRTPROCESS)
	
elseif(LIBRTPROCESS)
	pkg_check_modules(LIBRTPROCESS REQUIRED rtprocess)
	if (LIBRTPROCESS_FOUND)
		add_definitions(-DUSE_LIBRTPROCESS)
		include_directories(${LIBRTPROCESS_INCLUDE_DIR})
		target_link_libraries(rawproc ${LIBRTPROCESS_LIBRARIES})
		target_link_libraries(img ${LIBRTPROCESS_LIBRARIES})
		target_link_libraries(exif ${LIBRTPROCESS_LIBRARIES})
	else()
		message(STATUS "librtprocess not found")
	endif(LIBRTPROCESS_FOUND)
endif()


#lensfun dbupdate, require libcurl and libarchive:
if(LENSFUN_DBUPDATE)
	message(STATUS "including libcurl and libarchive for LENSFUN_DBUPDATE")
	
	pkg_check_modules(LIBCURL REQUIRED libcurl)
	if (LIBCURL_FOUND)
		include_directories(${LIBCURL_INCLUDE_DIR})
		target_link_libraries(rawproc ${LIBCURL_LIBRARIES})
	else()
		message(STATUS "libcurl not found")
	endif()
	pkg_check_modules(LIBARCHIVE REQUIRED libarchive)
	if (LIBARCHIVE_FOUND)
		include_directories(${LIBARCHIVE_INCLUDE_DIR})
		target_link_libraries(rawproc ${LIBARCHIVE_LIBRARIES})
	else()
		message(STATUS "libarchive not found")
	endif()
	
	add_definitions(-DCURL_STATICLIB -DUSE_LENSFUN_DBUPDATE)
	
endif(LENSFUN_DBUPDATE)

if(WIN32)
	target_link_libraries(rawproc wsock32 ws2_32 psapi)
	target_link_libraries(img wsock32 ws2_32 psapi)
	target_link_libraries(exif wsock32 ws2_32 psapi)
endif(WIN32)

add_subdirectory(src)

