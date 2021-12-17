#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "mp4v2/mp4v2.h"

// 编译时Makefile里控制
#ifdef ENABLE_DEBUG
	#define DEBUG(fmt, args...) 	printf(fmt, ##args)
#else
	#define DEBUG(fmt, args...)
#endif


int unpackMp4File(char *mp4FileName, char *videoFileName, char *audioFileName);


unsigned char g_sps[64] = {0};
unsigned char g_pps[64] = {0};
unsigned int  g_spslen  = 0;
unsigned int  g_ppslen  = 0;


int main(int argc, char **argv)
{
	if(argc < 2)
	{
		printf("Usage: \n"
			   "   %s ./avfile/test1.mp4 ./test1_out.h264 ./test1_out.aac\n"
			   "   %s ./avfile/test2.mp4 ./test2_out.h264 ./test2_out.aac\n",
				argv[0], argv[0]);
		return -1;
	}

    int ret = unpackMp4File(argv[1], argv[2], argv[3]);
	if(ret == 0)
	{
		printf("\033[32mSuccess!\033[0m\n");
	}
	else
	{
		printf("\033[31mFailed!\033[0m\n");
	}

    return 0;
}

int getH264Stream(MP4FileHandle mp4Handler, int videoTrackId, int totalSamples, char *saveFileName)
{
	// 调用的接口要传的参数
	uint32_t curFrameIndex = 1; // `MP4ReadSample`函数的参数要求是从1开始，但我们打印帧下标还是从0开始
    uint8_t *pData = NULL;
    uint32_t nSize = 0;
    MP4Timestamp pStartTime;
    MP4Duration pDuration;
    MP4Duration pRenderingOffset;
    bool pIsSyncSample = 0;

	// 写文件要用的参数
    char naluHeader[4] = {0x00, 0x00, 0x00, 0x01};
    FILE *fpVideo = NULL;

    if(!mp4Handler)
		return -1;

    fpVideo = fopen(saveFileName, "wb"); 
	if (fpVideo == NULL)
	{
		printf("open file(%s) error!\n", saveFileName);
		return -1;
	}
 
    while(curFrameIndex <= totalSamples)
    {   
        // 如果传入MP4ReadSample的视频pData是null，它内部就会new 一个内存
        // 如果传入的是已知的内存区域，则需要保证空间bigger then max frames size.
        MP4ReadSample(mp4Handler, videoTrackId, curFrameIndex, &pData, &nSize, &pStartTime, &pDuration, &pRenderingOffset, &pIsSyncSample);

		DEBUG("[\033[35mvideo\033[0m] ");

		if(pIsSyncSample)
		{
			DEBUG("IDR\t");

			fwrite(naluHeader, 4, 1, fpVideo);
			fwrite(g_sps, g_spslen, 1, fpVideo);

			fwrite(naluHeader, 4, 1, fpVideo);
			fwrite(g_pps, g_ppslen, 1, fpVideo);
		}
		else
		{
			DEBUG("SLICE\t");
		}

        if(pData && nSize > 4)
		{
			// `MP4ReadSample`函数的参数要求是从1开始，但我们打印帧下标还是从0开始；而大小已经包含了4字节的start code长度
			DEBUG("frame index: %d\t size: %d\n", curFrameIndex - 1, nSize);
            fwrite(naluHeader, 4, 1, fpVideo);
			fwrite(pData + 4, nSize - 4, 1, fpVideo); // pData+4了，那nSize就要-4
        }
        
        free(pData);
        pData = NULL;
		
        curFrameIndex++;
    }       
    fflush(fpVideo);
    fclose(fpVideo);  
 
    return 0;
}

int getAACStream(MP4FileHandle mp4Handler, int audioTrackId, int totalSamples, char *saveFileName)
{
	// 调用的接口要传的参数
	uint32_t curFrameIndex = 1; // `MP4ReadSample`函数的参数要求是从1开始，但我们打印帧下标还是从0开始
    uint8_t *pData = NULL;
    uint32_t nSize = 0;

	// 写文件要用的参数
	FILE *fpAudio = NULL;

	if(!mp4Handler)
		return -1;

	fpAudio = fopen(saveFileName, "wb");
	if (fpAudio == NULL)
	{
		printf("open file(%s) error!\n", saveFileName);
		return -1;
	}
 
	while(curFrameIndex <= totalSamples)
	{
		// 如果传入MP4ReadSample的音频pData是null，它内部就会new 一个内存
		// 如果传入的是已知的内存区域，则需要保证空间bigger then max frames size.
		MP4ReadSample(mp4Handler, audioTrackId, curFrameIndex, &pData, &nSize, NULL, NULL, NULL, NULL);

		DEBUG("[\033[36maudio\033[0m] ");

		if(pData)
		{			
			DEBUG("frame index: %d\t size: %d\n", curFrameIndex - 1, nSize);
			fwrite(pData, nSize, 1, fpAudio);
		}
		
		free(pData);
		pData = NULL;
		
		curFrameIndex++;
	}		
	fflush(fpAudio);
	fclose(fpAudio);  
 
	return 0;
}


int unpackMp4File(char *mp4FileName, char *videoFileName, char *audioFileName)
{
	MP4FileHandle mp4Handler = 0;
	uint32_t trackCnt = 0;	
	int videoTrackId = -1;
	int audioTrackId = -1;
	unsigned int videoSampleCnt = 0;
	unsigned int audioSampleCnt = 0;


	mp4Handler = MP4Read(mp4FileName);
	if (mp4Handler <= 0)
	{
		printf("MP4Read(%s) error!\n", mp4FileName);
		return -1;
	}
 
	trackCnt = MP4GetNumberOfTracks(mp4Handler, NULL, 0); //获取音视频轨道数
	printf("****************************\n");
	printf("trackCnt: %d\n", trackCnt);
 
    for (int i = 0; i < trackCnt; i++)
    {
		// 获取trackId，判断获取数据类型: 1-获取视频数据，2-获取音频数据
		MP4TrackId trackId = MP4FindTrackId(mp4Handler, i, NULL, 0);
		const char* trackType = MP4GetTrackType(mp4Handler, trackId);

		if (MP4_IS_VIDEO_TRACK_TYPE(trackType))
		{
			// 不关心，只是打印出来看看
			MP4Duration duration = 0;
			uint32_t timescale = 0;

			videoTrackId = trackId;

			duration = MP4GetTrackDuration(mp4Handler, videoTrackId);
			timescale = MP4GetTrackTimeScale(mp4Handler, videoTrackId);
			videoSampleCnt = MP4GetTrackNumberOfSamples(mp4Handler, videoTrackId);
			
			printf("video params: \n"
				   " - trackId: %d\n"
				   " - duration: %lu\n"
				   " - timescale: %d\n"
				   " - samples count: %d\n",
				   videoTrackId, duration, timescale, videoSampleCnt);

			// 读取 sps/pps 
			uint8_t **seqheader;			
			uint32_t *seqheadersize;
			uint8_t **pictheader;
			uint32_t *pictheadersize;

			MP4GetTrackH264SeqPictHeaders(mp4Handler, videoTrackId, &seqheader, &seqheadersize, &pictheader, &pictheadersize);

			// 获取sps
            for (int ix = 0; seqheadersize[ix] != 0; ix++)
            {
				memcpy(g_sps, seqheader[ix], seqheadersize[ix]);
				g_spslen = seqheadersize[ix];
				free(seqheader[ix]);
			}
			free(seqheader);
			free(seqheadersize);

			// 获取pps
			for (int ix = 0; pictheader[ix] != 0; ix++)
			{
				memcpy(g_pps, pictheader[ix], pictheadersize[ix]);
				g_ppslen = pictheadersize[ix];
				free(pictheader[ix]);
			}
			free(pictheader);
			free(pictheadersize);
        }
		else if (MP4_IS_AUDIO_TRACK_TYPE(trackType))
		{
			audioTrackId = trackId;
			audioSampleCnt = MP4GetTrackNumberOfSamples(mp4Handler, audioTrackId);

			printf("audio params: \n"
				   " - trackId: %d\n"
				   " - samples count: %d\n",
				   audioTrackId, audioSampleCnt);
        }
    }
	printf("****************************\n");

    // 解析完了mp4，主要是为了获取sps pps 还有video的trackID
    if(videoTrackId >= 0)
	{
        getH264Stream(mp4Handler, videoTrackId, videoSampleCnt, videoFileName);  
    }

	if(audioTrackId >= 0)
	{
        getAACStream(mp4Handler, audioTrackId, audioSampleCnt, audioFileName);
    }
 
    // 需要mp4close 否则在嵌入式设备打开mp4上多了会内存泄露挂掉.
    MP4Close(mp4Handler, 0);
	
    return 0;
}

