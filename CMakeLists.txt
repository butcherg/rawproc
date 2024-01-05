
cmake_minimum_required(VERSION 3.15)

MESSAGE("CMAKE_HOST_SYSTEM_NAME ${CMAKE_HOST_SYSTEM_NAME}")
MESSAGE("CMAKE_HOST_SYSTEM_VERSION ${CMAKE_HOST_SYSTEM_VERSION}")

project(rawproc LANGUAGES CXX C)

string(TIMESTAMP BUILDDATE)

add_compile_options(-ggdb -DVERSION="1.3"  -DBUILDDATE="${BUILDDATE}")
#add_link_options(-lz)

if(GMIC)
	add_compile_options(-DUSE_GMIC)
endif(GMIC)

if (LIBRTPROCESS)
	add_compile_options(-DUSE_LIBRTPROCESS)
endif(LIBRTPROCESS)

if (LENSFUN_DBUPDATE)
	add_compile_options(-DUSE_LENSFUN_DBUPDATE)
endif(LENSFUN_DBUPDATE)



if (LCMS2CFLAGS)
	add_compile_options(${LCMS2CFLAGS})
endif(LCMS2CFLAGS)

if (LIBRAWCFLAGS)
	add_compile_options(${LIBRAWCFLAGS})
endif(LIBRAWCFLAGS)

#if (LENSFUNCFLAGS)
#	add_compile_options(${LENSFUNCFLAGS})
#endif(LENSFUNCFLAGS)

if (EXIV2CFLAGS)
	add_compile_options(${EXIV2CFLAGS})
endif(EXIV2CFLAGS)

if (GMICCFLAGS)
	add_compile_options(${GMICCFLAGS})
endif(GMICCFLAGS)

if (LIBRTPROCESSFLAGS)
	add_compile_options(${LIBRTPROCESSFLAGS})
endif(LIBRTPROCESSFLAGS)

add_executable(rawproc src/rawprocApp.cpp)
add_executable(img src/img.cpp)
add_executable(wxcmd src/wxcmd.cpp)
add_executable(exif src/exif.cpp)

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
if (LCMS2INC AND LCMS2LIB)
	include_directories ("${LCMS2INC}")
	target_link_libraries(rawproc "${LCMS2LIB}")
	target_link_libraries(img "${LCMS2LIB}")
	target_link_libraries(exif "${LCMS2LIB}")
	if (LCMS2FLAGS)
		message(STATUS "LittleCMS2 hand-configured with LCMS2FLAGS, LCMS2INC and LCMS2LIB")
	else()
		message(STATUS "LittleCMS2 hand-configured with LCMS2INC and LCMS2LIB")
	endif (LCMS2FLAGS)
else()
	pkg_check_modules(LCMS2 REQUIRED lcms2)
	if (LCMS2_FOUND)
		include_directories(${LCMS2_INCLUDE_DIR})
		target_link_libraries(rawproc ${LCMS2_LIBRARIES})
		target_link_libraries(img ${LCMS2_LIBRARIES})
		target_link_libraries(exif ${LCMS2_LIBRARIES})
	else()
		message(STATUS "lcms2 not found")
	endif(LCMS2_FOUND)
endif(LCMS2INC AND LCMS2LIB)

#Libraw:
if (LIBRAWINC AND LIBRAWLIB)
	include_directories ("${LIBRAWINC}")
	target_link_libraries(rawproc "${LIBRAWLIB}")
	target_link_libraries(img "${LIBRAWLIB}")
	target_link_libraries(img "${LIBRAWLIB}")
	if (LIBRAWFLAGS)
		message(STATUS "Libraw hand-configured with LIBRAWFLAGS, LIBRAWINC and LIBRAWLIB")
	else()
		message(STATUS "Libraw hand-configured with LIBRAWINC and LIBRAWLIB")
	endif (LIBRAWFLAGS)
	
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
endif(LIBRAWINC AND LIBRAWLIB)

#Lensfun:
if (LENSFUNINC AND LENSFUNLIB)
	if (LENSFUNCFLAGS)
		target_compile_options(rawproc PRIVATE ${LENSFUNCFLAGS})
		target_compile_options(img PRIVATE ${LENSFUNCFLAGS})
		target_compile_options(exif PRIVATE ${LENSFUNCFLAGS})
	endif(LENSFUNCFLAGS)
	if (LENSFUNLFLAGS)
		target_link_options(rawproc PRIVATE ${LENSFUNLFLAGS})
		target_link_options(img PRIVATE ${LENSFUNLFLAGS})
		target_link_options(exif PRIVATE ${LENSFUNLFLAGS})
	endif(LENSFUNLFLAGS)
	include_directories ("${LENSFUNINC}")
	target_link_libraries(rawproc "${LENSFUNLIB}")
	target_link_libraries(img "${LENSFUNLIB}")
	target_link_libraries(exif "${LENSFUNLIB}")
	if (LENSFUNFLAGS)
		message(STATUS "Lensfun hand-configured with LENSFUNFLAGS, LENSFUNINC and LENSFUNLIB")
	else()
		message(STATUS "Lensfun hand-configured with LENSFUNINC and LENSFUNLIB")
	endif (LENSFUNFLAGS)
	
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
endif (LENSFUNINC AND LENSFUNLIB)

#exiv2:
if (EXIV2INC AND EXIV2LIB)
	include_directories ("${EXIV2INC}")
	target_link_libraries(rawproc "${EXIV2LIB}")
	target_link_libraries(img "${EXIV2LIB}")
	target_link_libraries(exif "${EXIV2LIB}")
	if (EXIV2FLAGS)
		message(STATUS "exiv2 hand-configured with EXIV2FLAGS, EXIV2INC and EXIV2LIB")
	else()
		message(STATUS "exiv2 hand-configured with EXIV2INC and EXIV2LIB")
	endif (EXIV2FLAGS)
	
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
endif (EXIV2INC AND EXIV2LIB)


#Optional Features:

#gmic:
if (GMIC)
	if (GMICINC AND GMICLIB)
		include_directories("${GMICINC}")
		target_link_libraries(rawproc "${GMICLIB}")
		target_link_libraries(img "${GMICLIB}")
		target_link_libraries(exif "${GMICLIB}")
		if (GMICFLAGS)
			message(STATUS "GMIC hand-configured with GMICFLAGS, GMICINC and GMICLIB")
		else()
			message(STATUS "GMIC hand-configured with GMICINC and GMICLIB")
		endif (GMICFLAGS)
		
	else()
		pkg_check_modules(GMIC REQUIRED gmic)
		if (GMIC_FOUND)
			include_directories(${GMIC_INCLUDE_DIR})
			target_link_libraries(rawproc ${GMIC_LIBRARIES})
			target_link_libraries(img ${GMIC_LIBRARIES})
			target_link_libraries(exif ${GMIC_LIBRARIES})
		else()
			message(STATUS "gmic not found")
		endif(GMIC_FOUND)
	endif (GMICINC AND GMICLIB)
endif (GMIC)

if (LIBRTPROCESS)
	if (LIBRTPROCESSINC AND LIBRTPROCESSLIB)
		include_directories("${LIBRTPROCESSINC}")
		target_link_libraries(rawproc "${LIBRTPROCESSLIB}")
		target_link_libraries(img "${LIBRTPROCESSLIB}")
		target_link_libraries(exif "${LIBRTPROCESSLIB}")
		if (LIBRTPROCESSFLAGS)
			message(STATUS "LIBRTPROCESSFLAGS hand-configured with LIBRTPROCESSFLAGS, LIBRTPROCESSINC and LIBRTPROCESSLIB")
		else()
			message(STATUS "LIBRTPROCESSFLAGS hand-configured with LIBRTPROCESSINC and LIBRTPROCESSLIB")
		endif (LIBRTPROCESSFLAGS)
		
	else()
		pkg_check_modules(LIBRTPROCESS REQUIRED rtprocess)
		if (LIBRTPROCESS_FOUND)
			include_directories(${LIBRTPROCESS_INCLUDE_DIR})
			target_link_libraries(rawproc ${LIBRTPROCESS_LIBRARIES})
			target_link_libraries(img ${LIBRTPROCESS_LIBRARIES})
			target_link_libraries(exif ${LIBRTPROCESS_LIBRARIES})
		else()
			message(STATUS "gmic not found")
		endif(LIBRTPROCESS_FOUND)
	endif (LIBRTPROCESSINC AND LIBRTPROCESSLIB)
endif (LIBRTPROCESS)

#lensfun dbupdate, require libcurl and libarchive:
if(LENSFUN_DBUPDATE)
	find_package(CURL REQUIRED)
	include_directories(${CURL_INCLUDE_DIR})
	target_link_libraries(rawproc ${CURL_LIBRARIES})
	
	find_package(LibArchive REQUIRED)
	include_directories(${LibArchive_INCLUDE_DIR})
	target_link_libraries(rawproc ${LibArchive_LIBRARIES})
endif(LENSFUN_DBUPDATE)

if(WIN32)
	target_link_libraries(rawproc wsock32 ws2_32)
	target_link_libraries(img wsock32 ws2_32)
	target_link_libraries(exif wsock32 ws2_32)
endif(WIN32)

if (LCMS2LFLAGS)
	add_link_options(${LCMS2LFLAGS})
endif(LCMS2LFLAGS)

if (LIBRAWLFLAGS)
	add_link_options(${LIBRAWLFLAGS})
endif(LIBRAWLFLAGS)

if (LENSFUNLFLAGS)
	add_link_options(${LENSFUNLFLAGS})
endif(LENSFUNLFLAGS)

if (EXIV2LFLAGS)
	add_link_options(${EXIV2LFLAGS})
endif(EXIV2LFLAGS)

if (GMICLFLAGS)
	add_link_options(${GMICLFLAGS})
endif(GMICLFLAGS)

if (LIBRTPROCESSLFLAGS)
	add_link_options(${LIBRTPROCESSLFLAGS})
endif(LIBRTPROCESSLFLAGS)