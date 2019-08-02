
#include <jni.h>
#include <string>
#include <malloc.h>
#include <cstring>
#include "gif_lib.h"
#include <android/log.h>
#include <android/bitmap.h>
#define  LOG_TAG    "wangyi"
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)
#define  argb(a, r, g, b) ( ((a) & 0xff) << 24 ) | ( ((b) & 0xff) << 16 ) | ( ((g) & 0xff) << 8 ) | ((r) & 0xff)

typedef struct GifBean {
//播放帧数  第几帧
    int current_frame;
    int total_frame;
//    延迟时间数组  长度不确定    malloc  帧  155   计算
    int *dealys;

} GifBean;

void drawFrame(GifFileType *pType, GifBean *pBean, AndroidBitmapInfo info, void *pVoid);

extern "C"
JNIEXPORT jlong JNICALL
Java_com_netease_ndk_1gif_GifHandler_loadPath(JNIEnv *env, jobject instance, jstring path_) {
    const char *path =  env->GetStringUTFChars(path_,0);
    int err;
    GifFileType *fileType = DGifOpenFileName(path, &err);
    DGifSlurp(fileType);
    GifBean *gifBean = (GifBean *)(malloc(sizeof(GifBean)));
    memset(gifBean,0, sizeof(GifBean));
    fileType->UserData = gifBean;
    gifBean->dealys = (int *)(malloc(sizeof(int) * fileType->ImageCount));
    memset(gifBean->dealys,0, sizeof(int)*fileType->ImageCount);
    fileType->UserData = gifBean;
    gifBean->current_frame =0;
    gifBean->total_frame = fileType->ImageCount;
    ExtensionBlock *ext;
    for (int i = 0; i < fileType->ImageCount; ++i) {
        SavedImage frame = fileType->SavedImages[i];
        for (int j = 0; j < frame.ExtensionBlockCount; ++j) {
            if(frame.ExtensionBlocks[j].Function == GRAPHICS_EXT_FUNC_CODE){
                ext = &frame.ExtensionBlocks[j];
                break;
            }
    }
        if(ext){
            int frame_delay = 10*(ext->Bytes[1] | ext->Bytes[2]<<8);
            LOGE("%d",frame_delay);
            gifBean->dealys[i] = frame_delay;
        }

    }

    env->ReleaseStringUTFChars(path_,path);
    return (jlong)(fileType);

}

void drawFrame(GifFileType *gifFileType, GifBean *gifBean, AndroidBitmapInfo info, void *pixels) {


//  当前帧
    SavedImage savedImage = gifFileType->SavedImages[gifBean->current_frame];
    //整幅图片的首地址
    int *px = (int *) pixels;
    int pointPixel;
    GifImageDesc frameInfo = savedImage.ImageDesc;
    GifByteType gifByteType;//压缩数据
//    rgb数据     压缩工具
    ColorMapObject *colorMapObject = frameInfo.ColorMap;
//    Bitmap 往下便宜
    px = (int *) ((char *) px + info.stride * frameInfo.Top);
    //    每一行的首地址
    int *line;
    for (int y = frameInfo.Top; y < frameInfo.Top + frameInfo.Height; ++y) {
        line = px;
        for (int x = frameInfo.Left; x < frameInfo.Left + frameInfo.Width; ++x) {
//            拿到每一个坐标的位置  索引    ---》  数据
            pointPixel = (y - frameInfo.Top) * frameInfo.Width + (x - frameInfo.Left);
//            索引   rgb   LZW压缩  字典   （）缓存在一个字典
//解压
            gifByteType = savedImage.RasterBits[pointPixel];
            GifColorType gifColorType = colorMapObject->Colors[gifByteType];
            line[x] = argb(255, gifColorType.Red, gifColorType.Green, gifColorType.Blue);
        }
        px = (int *) ((char *) px + info.stride);
    }


}

extern "C"
JNIEXPORT jint JNICALL
Java_com_netease_ndk_1gif_GifHandler_getWidth__J(JNIEnv *env, jobject instance, jlong ndkGif) {

    // TODO
    GifFileType *gifFileType = (GifFileType *) ndkGif;
    return gifFileType->SWidth;
}extern "C"
JNIEXPORT jint JNICALL
Java_com_netease_ndk_1gif_GifHandler_getHeight__J(JNIEnv *env, jobject instance, jlong ndkGif) {

    // TODO
    GifFileType *gifFileType = (GifFileType *) ndkGif;
    return gifFileType->SHeight;

}

extern "C"
JNIEXPORT jint JNICALL
Java_com_netease_ndk_1gif_GifHandler_updateFrame__JLandroid_graphics_Bitmap_2(JNIEnv *env,
                                                                              jobject instance,
                                                                              jlong ndkGif,
                                                                              jobject bitmap) {
    GifFileType *gifFileType = (GifFileType *)(ndkGif);
    GifBean *gifBean = (GifBean *) gifFileType->UserData;
    AndroidBitmapInfo info;
//    入参  出出参对象

//    像素数组
    AndroidBitmap_getInfo(env, bitmap, &info);
//    空的 gif --Bitmap
    void *pixels;
    AndroidBitmap_lockPixels(env, bitmap, &pixels);
    drawFrame(gifFileType, gifBean, info, pixels);
    gifBean->current_frame += 1;
    if (gifBean->current_frame >= gifBean->total_frame - 1) {
        gifBean->current_frame = 0;
        LOGE("重新过来  %d  ", gifBean->current_frame);
    }

    AndroidBitmap_unlockPixels(env, bitmap);
    return gifBean->dealys[gifBean->current_frame];
}