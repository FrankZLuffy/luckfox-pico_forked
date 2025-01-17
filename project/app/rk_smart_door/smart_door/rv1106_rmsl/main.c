// Copyright 2022 Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "smart_door.h"
#include "uart.h"
#include "isp.h"
#include "wifi_cmd.h"
#include "face_recognition.h"
#include "rkaiq_3A_server.h"
#include "param.h"

#include "v4l2_isp_luma.h"

#define ENABLE_GET_STREAM

#ifdef ENABLE_GET_STREAM

#define SAVE_ENC_FRM_CNT_MAX     30

static RK_U64 TEST_COMM_GetNowUs()
{
    struct timespec time = {0, 0};
    clock_gettime(CLOCK_MONOTONIC, &time);
    return (RK_U64)time.tv_sec * 1000000 + (RK_U64)time.tv_nsec / 1000; /* microseconds */
}

// fast client to get media buffer.
static void  *GetMediaBuffer0(void *arg)
{
    (void)arg;
    void *pData = RK_NULL;
    int loopCount = 0;
    int s32Ret;
    static bool quit = false;
    VENC_STREAM_S stFrame;

    FILE *fp = fopen("/tmp/pts.txt", "wb");
    if (!fp) {
        return NULL;
    }

    FILE *venc0_file = fopen("/tmp/venc-test.h264", "w");
    if (!venc0_file) {
        return NULL;
    }

    stFrame.pstPack = malloc(sizeof(VENC_PACK_S));

    while (!quit) {
        s32Ret = RK_MPI_VENC_GetStream(0, &stFrame, 1000);
        if (s32Ret == RK_SUCCESS) {
            if (venc0_file) {
                pData = RK_MPI_MB_Handle2VirAddr(stFrame.pstPack->pMbBlk);
                fwrite(pData, 1, stFrame.pstPack->u32Len, venc0_file);
                fflush(venc0_file);
            }
            RK_U64 nowUs = TEST_COMM_GetNowUs();

            printf("chn:0, loopCount:%d enc->seq:%d wd:%d pts=%lld delay=%lldus\n", loopCount,
                   stFrame.u32Seq, stFrame.pstPack->u32Len, stFrame.pstPack->u64PTS, nowUs - stFrame.pstPack->u64PTS);
            if (fp) {
                char str[128];
                snprintf(str, sizeof(str), "seq:%u, pts:%llums\n", stFrame.u32Seq, stFrame.pstPack->u64PTS / 1000);
                fputs(str, fp);
                fsync(fileno(fp));
            }

            s32Ret = RK_MPI_VENC_ReleaseStream(0, &stFrame);
        } else {
            printf("RK_MPI_VENC_GetStream timeout %x\n", s32Ret);
        }

        if (++loopCount >= SAVE_ENC_FRM_CNT_MAX) {
            quit = true;
            break;
        }

        usleep(10 * 1000);
    }

    if (venc0_file)
        fclose(venc0_file);

    if (fp)
        fclose(fp);

    free(stFrame.pstPack);
    return NULL;
}
#endif

int main(int argc, char *argv[]) {
    FILE *fp = NULL;
    const char *ini_file = "/tmp/smart_door.ini";
    const char *sensor_name = "gc2093";
#if SC035_VGA_SENSOR
    char *face_sensor_name = "m01_b_sc035gs 3-0030";
#elif SC2355_SENSOR
    char *face_sensor_name = "m01_b_sc2355 3-0030";
#elif SC132GS_SENSOR
    char *face_sensor_name = "m01_b_sc132gs 3-0030";
#else
#error "plx config sensor in CMakeLists.txt"
#endif
    char *face_mdev_path_cif_ir = "/dev/media1";
    char *face_mdev_path_isp_flood = "/dev/media3";
    char *face_mdev_path_isp_pro = "/dev/media4";
    char *param_ini_path = NULL; // use default param init file
    char cmd_line[20];
    int cam_id = 0;
    void *ctx = NULL;
    pthread_t pipe0_thread = 0, uvc_thread = 0, face_thread = 0;
    struct face_recogniton_arg face_arg;

    face_arg.sensor_name = face_sensor_name;
    face_arg.mdev_path_cif_ir = face_mdev_path_cif_ir;
    face_arg.mdev_path_isp_flood = face_mdev_path_isp_flood;
    face_arg.mdev_path_isp_pro = face_mdev_path_isp_pro;

    if (RK_MPI_SYS_Init() != RK_SUCCESS) {
        goto exit_mpi_sys;
    }

    if (rk_param_init(param_ini_path)) {
        RK_LOGE("rk_param_init failed!");
        goto exit_param;
    }

    if (rk_isp_configure_media_pipeline(cam_id)) {
        RK_LOGE("rk_isp_configure_media_pipeline failed!");
        goto exit_param;
    }

    ctx = rkaiq_3a((char *)sensor_name, true);
    if (!ctx) {
        RK_LOGE("rkaiq_3a failed!");
        goto exit_param;
    }

    if (rk_isp_setExpSwAttr(cam_id, ctx)) {
        RK_LOGE("rk_isp_set_exp failed!");
        goto exit_param;
    }

#ifdef ENABLE_GET_STREAM
    GetMediaBuffer0(NULL);
#endif

    if ((fp = fopen(ini_file, "w+")) == NULL) {
        RK_LOGE("open ini file failed!");
        goto exit_param;
    }

    while(1) {
        memset(cmd_line, 0, sizeof(cmd_line));
        fseek(fp, 0, SEEK_SET);
        fgets(cmd_line, 20, fp);
        truncate(ini_file, 0);

        if (strstr(cmd_line, "rtsp_start")) {
            if (pipe0_thread)
                continue;

            RK_LOGE("rtsp running");
            pthread_create(&pipe0_thread, 0, (void *)smart_door_pipe_rtsp_start, NULL);
        } else if (strstr(cmd_line, "rtsp_stop")) {
            if (!pipe0_thread)
                continue;

            RK_LOGE("rtsp stop");
            smart_door_pipe_rtsp_stop();
            pthread_join(pipe0_thread, NULL);
            pipe0_thread = 0;
        } else if (strstr(cmd_line, "rtsp_res")) {
            if (!pipe0_thread)
                continue;

            RK_LOGE("rtsp set resolution");
            int width, height;
            sscanf(cmd_line, "rtsp_res:%d*%d", &width, &height);
            smart_door_pipe_rtsp_set_resolution(width, height);
        } else if (strstr(cmd_line, "ai")) {
            RK_LOGE("smart_door_pipe_ai_aenc running");
            smart_door_pipe_ai_aenc();
        } else if (strstr(cmd_line, "ao")) {
            RK_LOGE("smart_door_pipe_adec_ao running");
            smart_door_pipe_adec_ao();
        } else if (strstr(cmd_line, "face_auto")) {
            RK_LOGE("smart_door_face_auto_test running");
            smart_door_face_auto_test(&face_arg);
        } else if (strstr(cmd_line, "face_start")) {
            if (face_thread)
                continue;

            RK_LOGE("smart_door_face_recogniton_start running");
            pthread_create(&face_thread, 0, (void *)smart_door_face_recogniton_start, &face_arg);
        } else if (strstr(cmd_line, "face_stop")) {
            if (!face_thread)
                continue;

            RK_LOGE("smart_door_face_recogniton stop!");
            smart_door_face_recogniton_stop();
            pthread_join(face_thread, NULL);
            face_thread = 0;
        } else if (strstr(cmd_line, "uart")) {
            RK_LOGE("smart_door_uart running");
            smart_door_uart();
        } else if (strstr(cmd_line, "uvc_start")) {
            if (uvc_thread)
                continue;

            RK_LOGE("smart_door uvc start!");
            pthread_create(&uvc_thread, 0, (void *)uvc_start, NULL);
        } else if (strstr(cmd_line, "uvc_stop")) {
            if (!uvc_thread)
                continue;

            RK_LOGE("smart_door uvc stop!");
            uvc_stop();
            pthread_join(uvc_thread, NULL);
            uvc_thread = 0;
        } else if (strstr(cmd_line, "wifi")) {
            RK_LOGE("smart_door wifi cmd running");
            smart_door_wifi_cmd();
        } else if (strstr(cmd_line, "exit")) {
            RK_LOGE("smart_door exit!");
            remove(ini_file);
            break;
        } else {
            usleep(1000000llu);
        }
    }

exit_param:
    rk_param_deinit();

exit_mpi_sys:
    RK_MPI_SYS_Exit();

    if (fp) {
        fclose(fp);
        fp = NULL;
    }

    return RK_SUCCESS;
}

