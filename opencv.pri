#--------------------------------------------------------OPENCV----------------------------------------------------
win32 {
    #Specify a path to the build directory of opencv library and library version
    OPENCV_VERSION = 450
    OPENCV_DIR = C:/Programming/3rdParties/opencv$${OPENCV_VERSION}/build
    INCLUDEPATH += $${OPENCV_DIR}/include

    #Specify the part of OpenCV path corresponding to compiler version
    win32-msvc2010: OPENCV_COMPILER_DIR = vc10
    win32-msvc2012: OPENCV_COMPILER_DIR = vc11
    win32-msvc2013: OPENCV_COMPILER_DIR = vc12
    win32-msvc2015: OPENCV_COMPILER_DIR = vc14
    win32-g++:      OPENCV_COMPILER_DIR = mingw

    #Specify the part of OpenCV path corresponding to target architecture
    win32:contains(QMAKE_TARGET.arch, x86_64){
        OPENCV_ARCHITECTURE_DIR = x64
    } else {
        OPENCV_ARCHITECTURE_DIR = x86
    }
    win32-g++: OPENCV_ARCHITECTURE_DIR = x86
    message( - opencv\'s binaries for arch \'$${OPENCV_ARCHITECTURE_DIR}\' were selected)

    #A tricky way to resolve debug and release library versions
    defineReplace(qtLibraryName) {
       unset(LIBRARY_NAME)
       LIBRARY_NAME = $$1
       CONFIG(debug, debug|release): RET = $$member(LIBRARY_NAME, 0)d
       isEmpty(RET):RET = $$LIBRARY_NAME
       return($$RET)
    }

    #Specify path to *.lib files
    win32-msvc*:LIBS += -L$${OPENCV_DIR}/$${OPENCV_ARCHITECTURE_DIR}/$${OPENCV_COMPILER_DIR}/lib/
    win32-msvc*:LIBS += -L$${OPENCV_DIR}/$${OPENCV_ARCHITECTURE_DIR}/$${OPENCV_COMPILER_DIR}/bin/
    win32-g++:  LIBS += -L$${OPENCV_DIR}/$${OPENCV_ARCHITECTURE_DIR}/$${OPENCV_COMPILER_DIR}/bin/

    #Specify names of *.lib files
    LIBS += -l$$qtLibraryName(opencv_core$${OPENCV_VERSION}) \
            -l$$qtLibraryName(opencv_imgproc$${OPENCV_VERSION}) \
            -l$$qtLibraryName(opencv_imgcodecs$${OPENCV_VERSION})

    message(OpenCV library version $${OPENCV_DIR}/$${OPENCV_ARCHITECTURE_DIR}/$${OPENCV_COMPILER_DIR} will be used)

} else {

    OPENCV_INSTALL_PATH = /home/alex/Programming/3rdParties/opencv/install

    INCLUDEPATH += $${OPENCV_INSTALL_PATH}/include/opencv4

    LIBS += -L$${OPENCV_INSTALL_PATH}/lib

    LIBS += -lopencv_core \
            -lopencv_imgproc \
            -lopencv_imgcodecs
}
