typedef unsigned short uint16;
typedef unsigned int uint32;
#pragma pack(push, 1)
// pcm文件头
typedef struct{
    uint32 ChunkID;             //00H 4 char "RIFF"标志
    uint32 ChunkSize;           //04H 4 long int 文件长度 文总长-8
    uint32 Format;              //08H 4 char "WAVE"标志
    uint32 SubChunk1ID;         //0CH 4 char "fmt "标志
    uint32 SubChunk1Size;       //10H 4 0x10000000H(PCM)过渡字节(不定)
    uint16 AudioFormat;         //14H 2 int 格式类别（0x01H为PCM形式的声音数据) 0x0100H
    uint16 NumChannels;         //16H 2 int 通道数，单声道为1，双声道为2
    uint32 SampleRate;          //18H 4 int 采样率（每秒样本数），表示每个通道的播放速度，
    uint32 ByteRate;            //1CH 4 long int 波形音频数据传送速率，其值Channels×SamplesPerSec×BitsPerSample/8
    uint16 BlockAlign;          //20H 2 int 数据块的调整数（按字节算的），其值为Channels×BitsPerSample/8
    uint16 BitsPerSample;       //22H 2 每样本的数据位数，表示每个声道中各个样本的数据位数。如果有多个声道，对每个声道而言，样本大小都一样。
    uint32 DataTag;           	//24H 4 char 数据标记符＂data＂
    uint32 DataLen;		//28H 4 long int 语音数据的长度(文长-44)
}PCM_HEAD, *PPCM_HEAD;

// law文件头
typedef struct{
    uint32 ChunkID;             //00H 4 char "RIFF"标志
    uint32 ChunkSize;           //04H 4 long int 文件长度 文总长-8
    uint32 Format;              //08H 4 char "WAVE"标志
    uint32 SubChunk1ID;         //0CH 4 char "fmt "标志
    uint32 SubChunk1Size;       //10H 4 0x12000000H(ALAW)
    uint16 AudioFormat;         //14H 2 int 格式类别 0x0600H
    uint16 NumChannels;         //16H 2 int 通道数，单声道为1，双声道为2
    uint32 SampleRate;          //18H 4 int 采样率（每秒样本数），表示每个通道的播放速度，
    uint32 ByteRate;            //1CH 4 long int 波形音频数据传送速率，其值Channels×SamplesPerSec×BitsPerSample/8
    uint16 BlockAlign;          //20H 2 int 数据块的调整数（按字节算的），其值为Channels×BitsPerSample/8
    uint32 BitsPerSample;       //22H 2 每样本的数据位数，表示每个声道中各个样本的数据位数。如果有多个声道，对每个声道而言，样本大小都一样。
    uint32 WaveFact;		//26H 4 char "fact"标志
    uint32 Temp1;		//2AH 4 0x04000000H
    uint32 Temp2;		//2EH 4 0x00530700H
    uint32 DataTag;           	//32H 4 char 数据标记符＂data＂
    uint32 DataLen;		//36H 4 long int 语音数据的长度(文长-58)
}LAW_HEAD, *PLAW_HEAD;

#pragma pack(pop)

#define	SIGN_BIT	0x80		/* Sign bit for a A-law byte. */
#define	QUANT_MASK	0xf		/* Quantization field mask. */
#define	NSEGS		8			/* Number of A-law segments. */
#define	SEG_SHIFT	4			/* Left shift for segment number. */
#define	SEG_MASK	0x70		/* Segment field mask. */
#define	BIAS		0x84		/* Bias for linear code. */
#define CLIP            8159

static const short seg_aend[8] = { 0x1F, 0x3F, 0x7F, 0xFF,
    0x1FF, 0x3FF, 0x7FF, 0xFFF };
static const short seg_uend[8] = { 0x3F, 0x7F, 0xFF, 0x1FF,
    0x3FF, 0x7FF, 0xFFF, 0x1FFF };
static int n = 0;
