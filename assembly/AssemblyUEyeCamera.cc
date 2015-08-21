#include <iostream>

#include <QApplication>

#include <nqlogger.h>

#include "AssemblyUEyeCamera.h"

AssemblyUEyeCameraEventThread::AssemblyUEyeCameraEventThread()
{
    cameraHandle_ = 0;
    runEventThread_ = false;
}

AssemblyUEyeCameraEventThread::~AssemblyUEyeCameraEventThread()
{

}

int AssemblyUEyeCameraEventThread::start(HIDS cameraHandle)
{
    int ret = 0;

    cameraHandle_ = cameraHandle;

    ret = is_EnableEvent(cameraHandle_, IS_SET_EVENT_FRAME);
    if (ret==0) {
        NQLog("AssemblyUEyeCameraEventThread::start()", NQLog::Message) << "is_EnableEvent " << ret;
        runEventThread_ = true;
        QThread::start ();
    }

    return ret;
}

void AssemblyUEyeCameraEventThread::stop ()
{
    runEventThread_ = false;
}

void AssemblyUEyeCameraEventThread::run ()
{
    while (runEventThread_) {
        if (is_WaitEvent (cameraHandle_, IS_SET_EVENT_FRAME, 1000) == IS_SUCCESS) {
            emit eventHappened();
        }
    }

    runEventThread_ = false;
    is_DisableEvent (cameraHandle_, IS_SET_EVENT_FRAME);
}

AssemblyUEyeCamera::AssemblyUEyeCamera(QObject *parent)
    : AssemblyVUEyeCamera(parent)
{
    cameraOpen_ = false;

    bufferProps_.width = 0;
    bufferProps_.height = 0;
    bufferProps_.bitspp = 8;
    bufferProps_.colorformat = IS_CM_MONO8;
    bufferProps_.imgformat = QImage::Format_Invalid;
    bufferProps_.pRgbTable = NULL;
    bufferProps_.tableentries = 0;

    for (int i = 0; i < 256; i++)
        table_[i] = qRgb (i, i, i);

    ZeroMemory(images_, sizeof(images_));
}

AssemblyUEyeCamera::~AssemblyUEyeCamera()
{
    if (cameraOpen_) close();
}

void AssemblyUEyeCamera::open()
{
    if (cameraOpen_) return;

    cameraHandle_ = getDeviceID();
    unsigned int ret = is_InitCamera(&cameraHandle_, 0);
    NQLog("AssemblyUEyeCamera::initialize()", NQLog::Message) << "is_InitCamera " << ret;

    CAMINFO* cameraInfo = new CAMINFO;
    if (!is_GetCameraInfo(cameraHandle_, cameraInfo)) {

        setID(cameraInfo->ID);
        setVersion(cameraInfo->Version);
        setDate(cameraInfo->Date);

        emit cameraInformationChanged();

    } else {
        NQLog("AssemblyUEyeCamera", NQLog::Fatal) << "cannot read camera information";
    }

    delete cameraInfo;

    SENSORINFO* sensorInfo = new SENSORINFO;
    if (!is_GetSensorInfo(cameraHandle_, sensorInfo)) {

        setSensorName(sensorInfo->strSensorName);
        setColorMode(sensorInfo->nColorMode);
        setMaxWidth(sensorInfo->nMaxWidth);
        setMaxHeight(sensorInfo->nMaxHeight);
        setMasterGain(sensorInfo->bMasterGain);
        setRedGain(sensorInfo->bRGain);
        setGreenGain(sensorInfo->bGGain);
        setBlueGain(sensorInfo->bBGain);
        setGlobalShutter(sensorInfo->bGlobShutter);
        setPixelSize(sensorInfo->wPixelSize);

        int colormode = 0;
        if (sensorInfo->nColorMode >= IS_COLORMODE_BAYER)
        {
            colormode = IS_CM_BGRA8_PACKED;
        }
        else
        {
            colormode = IS_CM_MONO8;
        }

        if (is_SetColorMode(cameraHandle_, colormode) != IS_SUCCESS) {
            NQLog("AssemblyUEyeCamera", NQLog::Fatal) << "cannot set color mode";
        }

        emit cameraInformationChanged();

    } else {
        NQLog("AssemblyUEyeCamera", NQLog::Fatal) << "cannot read sensor information";
    }

    delete sensorInfo;

    unsigned int uInitialParameterSet = IS_CONFIG_INITIAL_PARAMETERSET_NONE;

    if (is_Configuration(IS_CONFIG_INITIAL_PARAMETERSET_CMD_GET, &uInitialParameterSet, sizeof(unsigned int))) {
        NQLog("AssemblyUEyeCamera", NQLog::Fatal) << "cannot read sensor initial parameter set";
    }

    if (uInitialParameterSet == IS_CONFIG_INITIAL_PARAMETERSET_NONE) {
        ret = is_ResetToDefault(cameraHandle_);
    }

    ZeroMemory(&cameraProps_, sizeof(cameraProps_));

    // If the camera does not support a continuous AOI -> it uses special image formats
    cameraProps_.bUsesImageFormats = false;
    int nAOISupported = 0;
    if (is_ImageFormat(cameraHandle_, IMGFRMT_CMD_GET_ARBITRARY_AOI_SUPPORTED, (void*)&nAOISupported, sizeof(nAOISupported)) == IS_SUCCESS) {
        cameraProps_.bUsesImageFormats = (nAOISupported == 0);
    }

    if (cameraProps_.bUsesImageFormats) {
        // search the default formats
        cameraProps_.nImgFmtNormal  = searchDefaultImageFormats(CAPTMODE_FREERUN | CAPTMODE_SINGLE);
        cameraProps_.nImgFmtDefaultNormal = cameraProps_.nImgFmtNormal;
        cameraProps_.nImgFmtTrigger = searchDefaultImageFormats(CAPTMODE_TRIGGER_SOFT_SINGLE);
        cameraProps_.nImgFmtDefaultTrigger = cameraProps_.nImgFmtTrigger;

        // set the default formats
        if (!is_ImageFormat(cameraHandle_, IMGFRMT_CMD_SET_FORMAT, (void*)&cameraProps_.nImgFmtNormal, sizeof(cameraProps_.nImgFmtNormal))) {
            //m_nImageFormat = nFormat;
            //bRet = TRUE;
        }
    }

    setupCapture();

    eventThread_ = new AssemblyUEyeCameraEventThread();
    connect(eventThread_, SIGNAL(eventHappened()),
            this, SLOT(eventHappend()));
    eventThread_->start(cameraHandle_);

    cameraOpen_ = true;

    emit cameraOpened();
}

void AssemblyUEyeCamera::close()
{
    if (!cameraOpen_) return;

    eventThread_->stop();

    freeImages();

    if (eventThread_->wait(2000) == FALSE)
        eventThread_->terminate();

    unsigned int ret = is_ExitCamera(cameraHandle_);
    NQLog("AssemblyUEyeCamera::exit()", NQLog::Message) << "is_ExitCamera "  << ret;

    cameraOpen_ = false;

    emit cameraClosed();
}

void AssemblyUEyeCamera::updateInformation()
{
    if (!cameraOpen_) return;

    CAMINFO* cameraInfo = new CAMINFO;
    if (!is_GetCameraInfo(cameraHandle_, cameraInfo)) {

        setID(cameraInfo->ID);
        setVersion(cameraInfo->Version);
        setDate(cameraInfo->Date);

    } else {
        NQLog("AssemblyUEyeCamera", NQLog::Fatal) << "cannot read camera information";
    }

    delete cameraInfo;

    SENSORINFO* sensorInfo = new SENSORINFO;
    if (!is_GetSensorInfo(cameraHandle_, sensorInfo)) {

        setSensorName(sensorInfo->strSensorName);
        setColorMode(sensorInfo->nColorMode);
        setMaxWidth(sensorInfo->nMaxWidth);
        setMaxHeight(sensorInfo->nMaxHeight);
        setMasterGain(sensorInfo->bMasterGain);
        setRedGain(sensorInfo->bRGain);
        setGreenGain(sensorInfo->bGGain);
        setBlueGain(sensorInfo->bBGain);
        setGlobalShutter(sensorInfo->bGlobShutter);
        setPixelSize(sensorInfo->wPixelSize);

    } else {
        NQLog("AssemblyUEyeCamera", NQLog::Fatal) << "cannot read sensor information";
    }

    delete sensorInfo;
}

bool AssemblyUEyeCamera::isAvailable() const
{
    return (bool)IS_CAMERA_AVAILABLE(getStatus());
}

void AssemblyUEyeCamera::eventHappend()
{
    NQLog("AssemblyUEyeCamera::eventHappend()", NQLog::Message) << "new frame available";

    INT dummy = 0;
    char *pLast = NULL, *pMem = NULL;

    is_GetActSeqBuf(cameraHandle_, &dummy, &pMem, &pLast);
    lastBuffer_ = pLast;

    int nNum;
    INT ret = IS_SUCCESS;

    if (!cameraHandle_) return;

    if (bufferProps_.width < 1 || bufferProps_.height < 1)
        return;

    nNum = getImageNumber(lastBuffer_);
    ret = is_LockSeqBuf (cameraHandle_, nNum, lastBuffer_);
    image_ = QImage((uchar*)lastBuffer_,
                    bufferProps_.width,
                    bufferProps_.height,
                    bufferProps_.imgformat);
    QVector<QRgb> rgbTable;
    for (int i=0;i<bufferProps_.tableentries;++i) {
        rgbTable.push_back(bufferProps_.pRgbTable[i]);
    }
    image_.setColorCount(bufferProps_.tableentries);
    image_.setColorTable(rgbTable);

    // get current colormode
    int colormode = is_SetColorMode(cameraHandle_, IS_GET_COLOR_MODE);
    switch (colormode)
    {
    default:
        break;
    case IS_CM_BGR8_PACKED:
        //swap the R and B color parts, cause QImage::Format_RGB888 takes order RGB
        image_ = image_.rgbSwapped();
        break;
    }

    is_UnlockSeqBuf(cameraHandle_, nNum, lastBuffer_);

    emit imageAcquired(&image_);
}

void AssemblyUEyeCamera::acquireImage()
{
    if (!cameraOpen_) return;

    NQLog("AssemblyUEyeCamera::aquireImage()", NQLog::Message) << "is_FreezeVideo";
    unsigned int ret = is_FreezeVideo(cameraHandle_, IS_DONT_WAIT);
    NQLog("AssemblyUEyeCamera::aquireImage()", NQLog::Message) << "is_FreezeVideo " << ret;
}

int AssemblyUEyeCamera::searchDefaultImageFormats(int supportMask)
{
    int ret = IS_SUCCESS;
    int nNumber;
    int format = 0;
    IMAGE_FORMAT_LIST *pFormatList;
    IS_RECT rectAOI;

    if ((ret=is_ImageFormat(cameraHandle_, IMGFRMT_CMD_GET_NUM_ENTRIES, (void*)&nNumber, sizeof(nNumber))) == IS_SUCCESS &&
        (ret=is_AOI(cameraHandle_, IS_AOI_IMAGE_GET_AOI, (void*)&rectAOI, sizeof(rectAOI))) == IS_SUCCESS) {

        int i = 0;
        int nSize = sizeof(IMAGE_FORMAT_LIST) + (nNumber - 1) * sizeof(IMAGE_FORMAT_LIST);
        pFormatList = (IMAGE_FORMAT_LIST*)(new char[nSize]);
        pFormatList->nNumListElements = nNumber;
        pFormatList->nSizeOfListEntry = sizeof(IMAGE_FORMAT_INFO);

        if ((ret=is_ImageFormat(cameraHandle_, IMGFRMT_CMD_GET_LIST, (void*)pFormatList, nSize)) == IS_SUCCESS) {
            for (i=0; i<nNumber; i++) {
                if ((pFormatList->FormatInfo[i].nSupportedCaptureModes & supportMask) &&
                    pFormatList->FormatInfo[i].nHeight == (UINT)rectAOI.s32Height &&
                    pFormatList->FormatInfo[i].nWidth  == (UINT)rectAOI.s32Width)
                {
                    format = pFormatList->FormatInfo[i].nFormatID;
                    break;
                }
            }
        }
    }

    return format;
}

void AssemblyUEyeCamera::setupCapture()
{
    int width, height;

    // init the memorybuffer properties
    ZeroMemory(&bufferProps_, sizeof(bufferProps_));

    IS_RECT rectAOI;
    INT nRet = is_AOI(cameraHandle_, IS_AOI_IMAGE_GET_AOI, (void*)&rectAOI, sizeof(rectAOI));

    if (nRet == IS_SUCCESS) {
        width  = rectAOI.s32Width;
        height = rectAOI.s32Height;

        // get current colormode
        int colormode = is_SetColorMode(cameraHandle_, IS_GET_COLOR_MODE);

        if(colormode == IS_CM_BGR5_PACKED) {
            is_SetColorMode(cameraHandle_, IS_CM_BGR565_PACKED);
            colormode = IS_CM_BGR565_PACKED;
        }

        ZeroMemory(&bufferProps_, sizeof(bufferProps_));
        bufferProps_.width  = width;
        bufferProps_.height = height;
        bufferProps_.colorformat = colormode;
        bufferProps_.bitspp = getBitsPerPixel(colormode);

        switch (colormode)
        {
        default:
        case IS_CM_MONO8:
        case IS_CM_SENSOR_RAW8:
            bufferProps_.pRgbTable = table_;
            bufferProps_.tableentries = 256;
            bufferProps_.imgformat = QImage::Format_Indexed8;
            break;
        case IS_CM_BGR565_PACKED:
            bufferProps_.imgformat = QImage::Format_RGB16;
            break;
        case IS_CM_RGB8_PACKED:
        case IS_CM_BGR8_PACKED:
            bufferProps_.imgformat = QImage::Format_RGB888;
            break;
        case IS_CM_RGBA8_PACKED:
        case IS_CM_BGRA8_PACKED:
            bufferProps_.imgformat = QImage::Format_RGB32;
            break;
        }

        allocImages();
    }
}

int AssemblyUEyeCamera::getBitsPerPixel(int colormode)
{
    switch (colormode)
    {
    default:
    case IS_CM_MONO8:
    case IS_CM_SENSOR_RAW8:
        return 8;   // occupies 8 Bit
    case IS_CM_MONO12:
    case IS_CM_MONO16:
    case IS_CM_SENSOR_RAW12:
    case IS_CM_SENSOR_RAW16:
    case IS_CM_BGR5_PACKED:
    case IS_CM_BGR565_PACKED:
    case IS_CM_UYVY_PACKED:
    case IS_CM_CBYCRY_PACKED:
        return 16;  // occupies 16 Bit
    case IS_CM_RGB8_PACKED:
    case IS_CM_BGR8_PACKED:
        return 24;
    case IS_CM_RGBA8_PACKED:
    case IS_CM_BGRA8_PACKED:
    case IS_CM_RGBY8_PACKED:
    case IS_CM_BGRY8_PACKED:
    case IS_CM_RGB10_PACKED:
    case IS_CM_BGR10_PACKED:
        return 32;
    }
}

bool AssemblyUEyeCamera::allocImages()
{
    lastBuffer_ = NULL;

    int nWidth = 0;
    int nHeight = 0;

    UINT nAbsPosX;
    UINT nAbsPosY;

    is_AOI(cameraHandle_, IS_AOI_IMAGE_GET_POS_X_ABS, (void*)&nAbsPosX , sizeof(nAbsPosX));
    is_AOI(cameraHandle_, IS_AOI_IMAGE_GET_POS_Y_ABS, (void*)&nAbsPosY , sizeof(nAbsPosY));

    is_ClearSequence(cameraHandle_);

    for (unsigned int i = 0; i < sizeof(images_) / sizeof(images_[0]); i++) {

        if (images_[i].pBuf) {
            is_FreeImageMem (cameraHandle_, images_[i].pBuf, images_[i].nImageID);
            images_[i].pBuf = NULL;
            images_[i].nImageID = 0;
        }

        nWidth = bufferProps_.width;
        nHeight = bufferProps_.height;

        if (nAbsPosX) {
            bufferProps_.width = nWidth = getMaxWidth();
        }
        if (nAbsPosY)
        {
            bufferProps_.height = nHeight = getMaxHeight();
        }

        if (is_AllocImageMem (cameraHandle_, nWidth, nHeight, bufferProps_.bitspp, &images_[i].pBuf,
                              &images_[i].nImageID) != IS_SUCCESS)
            return FALSE;
        if (is_AddToSequence (cameraHandle_, images_[i].pBuf, images_[i].nImageID) != IS_SUCCESS)
            return FALSE;

        images_[i].nImageSeqNum = i + 1;
        images_[i].nBufferSize = nWidth * nHeight * bufferProps_.bitspp / 8;
    }

    return TRUE;
}

bool AssemblyUEyeCamera::freeImages()
{
    lastBuffer_ = NULL;
    for (unsigned int i = 0; i < sizeof(images_) / sizeof(images_[0]); i++)
    {
        if (images_[i].pBuf)
        {
            is_FreeImageMem (cameraHandle_, images_[i].pBuf, images_[i].nImageID);
            images_[i].pBuf = NULL;
            images_[i].nImageID = 0;
        }
    }

    return true;
}

int AssemblyUEyeCamera::getImageNumber(char * pBuffer)
{
    for (unsigned int i = 0; i < sizeof(images_) / sizeof(images_[0]); i++)
        if (images_[i].pBuf == pBuffer)
            return images_[i].nImageSeqNum;

    return 0;
}
