include("D:/audioFileAnalyzer/audioFileAnalyzer/build/Desktop_Qt_6_9_1_MinGW_64_bit-Debug/.qt/QtDeploySupport.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/audioFileAnalyzer-plugins.cmake" OPTIONAL)
set(__QT_DEPLOY_I18N_CATALOGS "qtbase;qtmultimedia")

qt6_deploy_runtime_dependencies(
    EXECUTABLE D:/audioFileAnalyzer/audioFileAnalyzer/build/Desktop_Qt_6_9_1_MinGW_64_bit-Debug/audioFileAnalyzer.exe
    GENERATE_QT_CONF
)
