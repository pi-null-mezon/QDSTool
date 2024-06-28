win32 {
    DCMTK_INSTALL_PATH = C:/Programming/3rdParties/dcmtk-3.6.6/install
    LIBS += -L$${DCMTK_INSTALL_PATH}/bin
} else {
    DCMTK_INSTALL_PATH = /home/alex/Programming/3rdParties/dcmtk-3.6.6/install
}

INCLUDEPATH += $${DCMTK_INSTALL_PATH}/include

LIBS += -L$${DCMTK_INSTALL_PATH}/lib

LIBS += \
    -ldcmdata \
    -ldcmfg \
    -ldcmimgle \
    -loflog \
    -lofstd
