win32 {

} else {
    DCMTK_INSTALL_PATH = /home/alex/Programming/3rdParties/dcmtk/install

    INCLUDEPATH += $${DCMTK_INSTALL_PATH}/include

    LIBS += -L$${DCMTK_INSTALL_PATH}/lib

    LIBS += \
        #-lcmr \
        -ldcmdata \
        #-ldcmdsig \
        #-ldcmect \
        -ldcmfg \
        #-ldcmimage \
        -ldcmimgle \
        #-ldcmiod \
        #-ldcmjpeg \
        #-ldcmjpls \
        #-ldcmnet \
        #-ldcmpmap \
        #-ldcmpstat \
        #-ldcmqrdb \
        #-ldcmrt \
        #-ldcmseg \
        #-ldcmsr \
        #-ldcmtkcharls \
        #-ldcmtls \
        #-ldcmtract \
        #-ldcmwlm \
        #-ldcmxml \
        #-li2d \
        #-lijg12 \
        #-lijg16 \
        #-lijg8 \
        -loficonv \
        -loflog \
        -lofstd
}
