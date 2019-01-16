#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "p2au_au2p.h"

/*************** 检查输入信号在哪个压缩区间 *********************/
static short search(short val, short *table, short size)
{
    short i;
    for (i = 0; i < size; i++){
        if (val <= *table++){     //找出一个与输入信号最相近的值（大于）
            return i;
        }
    }
    return size;
}

/********************* PCM 2 a-law 的压缩 ****************************/
char linear2alaw(short  pcm_val){	/* 2's complement (16-bit range) */
    short mask;
    short seg;
    unsigned char aval;
    pcm_val = pcm_val >> 3;
    if (pcm_val >= 0){
        mask = 0xD5;		/* sign (7th) bit = 1 */
    }
    else{
        mask = 0x55;		/* sign bit = 0 */
        pcm_val = -pcm_val - 1;
    }
    seg = search(pcm_val, (short *)seg_aend, (short)8);
    if (seg >= 8){		/* out of range, return maximum value. */
        return (unsigned char)(0x7F ^ mask);
    }
    else{
        aval = (unsigned char)seg << SEG_SHIFT;
        if (seg < 2) {
            aval |= (pcm_val >> 1) & QUANT_MASK;
        }
        else{
            aval |= (pcm_val >> seg) & QUANT_MASK;
        }
        return (aval ^ mask);
    }
}

/***************** a-law 2 PCM *******************/
// alaw2linear() - Convert an A-law value to 16-bit linear PCM
short alaw2linear(unsigned char	a_val){
    short t;
    short seg;
    a_val ^= 0x55;
    t = (a_val & QUANT_MASK) << 4;
    seg = ((unsigned)a_val & SEG_MASK) >> SEG_SHIFT;
    switch (seg){
        case 0:
            t += 8;
            break;
        case 1:
            t += 0x108;
            break;
        default:
            t += 0x108;
            t <<= seg - 1;
            break;
    }
    return ((a_val & SIGN_BIT) ? t : -t);
}

/*********************** pcm 2 u law *************************/
char linear2ulaw(short pcm_val){	/* 2's complement (16-bit range) */
    short mask;
    short seg;
    unsigned char uval;
    pcm_val = pcm_val >> 2;
    if (pcm_val < 0){
        pcm_val = -pcm_val;
        mask = 0x7F;
    }
    else{
        mask = 0xFF;
    }
    if (pcm_val > CLIP){
        pcm_val = CLIP;		/* clip the magnitude */
    }
    pcm_val += (BIAS >> 2);
    seg = search(pcm_val, (short *)seg_uend, (short)8);
    if (seg >= 8){		/* out of range, return maximum value. */
        return (unsigned char)(0x7F ^ mask);
    }
    else{
        uval = (unsigned char)(seg << 4) | ((pcm_val >> (seg + 1)) & 0xF);
        return (uval ^ mask);
    }
}


// ulaw2linear() - Convert a u-law value to 16-bit linear PCM
short ulaw2linear(unsigned char	u_val){
    short t;
    u_val = ~u_val;
    t = ((u_val & QUANT_MASK) << 3) + BIAS;
    t <<= ((unsigned)u_val & SEG_MASK) >> SEG_SHIFT;
    return ((u_val & SIGN_BIT) ? (BIAS - t) : (t - BIAS));
}

void print_error(){
    printf("input Error!\n");
    printf("input argv[1] in filename\n");
    printf("input argv[2] 0=pcm:wav -> a-lam:wav\n");
    printf("              1=a-lam:wav -> pcm:wav\n");
    printf("              2=pcm:wav -> u-lam:wav\n");
    printf("              3=u-lam:wav -> pcm:wav\n");
}

void init_pcm_head(PPCM_HEAD head,PLAW_HEAD law){
    head->ChunkID = 0x46464952;
    head->ChunkSize = (law->ChunkSize - 50) * 2 + 36;
    head->Format = 0x45564157;
    head->SubChunk1ID = 0x20746d66;
    head->SubChunk1Size = 0x10;
    head->AudioFormat = 0x1;
    head->NumChannels = law->NumChannels ;
    head->SampleRate = law->SampleRate ;
    head->ByteRate = law->ByteRate * 2;
    head->BlockAlign = law->NumChannels * 2;
    head->BitsPerSample = 0x10;
    head->DataTag = 0x61746164;
    head->DataLen = law->DataLen * 2;
}

void init_law_head(PLAW_HEAD head,PPCM_HEAD pcm,char judge){
    head->ChunkID = 0x46464952;
    head->ChunkSize = (pcm->ChunkSize - 36) / 2 + 50;
    head->Format = 0x45564157;
    head->SubChunk1ID = 0x20746d66;
    head->SubChunk1Size = 0x12;
    if('0' == judge) head->AudioFormat = 0x6;
    if('2' == judge) head->AudioFormat = 0x7;
    head->NumChannels = pcm->NumChannels;
    head->SampleRate =  pcm->SampleRate;
    head->ByteRate = pcm->ByteRate / 2;
    head->BlockAlign = pcm->NumChannels;
    head->BitsPerSample = 0x08;
    head->WaveFact = 0x74636166;
    head->Temp1 = 0x4;
    head->Temp2 = (pcm->ChunkSize - 36) / 2;
    head->DataTag = 0x61746164;
    head->DataLen = (pcm->ChunkSize - 36) / 2;
}

void encoding(int argc, char judge, char* file_name, char* address){
    char buf[1024];
    FILE *fpin = NULL;
    FILE *fpout = NULL;
    PCM_HEAD pcm_head;
    LAW_HEAD law_head;
    char format[5] = ".wav";
    char c;
    short pcm_val;
    struct stat st;
    char output[64];
    sprintf(output,"%s%s","output//",file_name);
    if (argc != 3 || ('0' > judge || judge > '3')){
        print_error();
        exit(-1);
    }
    if ('0' == judge || '2' == judge){
        fpin = fopen(address, "r");
        if (NULL == fpin){
            printf("OpenFile Error!\n");
            exit(-1);
        }
        if (1 != fread(buf, sizeof(PCM_HEAD), 1, fpin)){
            printf("ReadFile Error!\n");
            goto END;
        }
        fpout = fopen(output, "w+");
        if (NULL == fpout){
            fclose(fpin);
            printf("OpenFile Error!\n");
            exit(-1);
        }
        PPCM_HEAD pcm = (PPCM_HEAD)&buf[0];
        if (pcm->AudioFormat != 0x1){
            printf("AudioFormat Error!\n");
            goto END;
        }
        init_law_head(&law_head,pcm,judge);
        fwrite(&law_head, sizeof(LAW_HEAD),1,fpout);
    }
    else if('1' == judge || '3' == judge){
        fpin = fopen(address, "r");
        if (NULL == fpin){
            printf("OpenFile Error!\n");
            exit(-1);
        }
        if (1 != fread(buf, sizeof(LAW_HEAD), 1, fpin)){
            printf("ReadFile Error!\n");
            goto END;
        }
        fpout = fopen(output, "w+");
        fpout = fopen(file_name, "w+");
        if (NULL == fpout){
            fclose(fpin);
            printf("OpenFile Error!\n");
            exit(-1);
        }
        PLAW_HEAD law = (PLAW_HEAD)&buf[0];
        if (law->AudioFormat != 0x6 && law->AudioFormat !=0x7){
            printf("AudioFormat Error!\n");
            goto END;
        }
        init_pcm_head(&pcm_head,law);
        fwrite(&pcm_head, sizeof(PCM_HEAD),1,fpout);
    }
    if ('0' == judge){	// pcm:wav -> a-law:wav
        while (1 == fread(&pcm_val, sizeof(short), 1, fpin)){
            c = linear2alaw(pcm_val);
            fputc(c, fpout);
        }
    }
    else if ('1' == judge){	// a-law:wav -> pcm:wav
        while (1 == fread(&c, sizeof(char), 1, fpin)){
            pcm_val = alaw2linear(c);
            fwrite(&pcm_val, sizeof(short), 1, fpout);
        }
    }
    else if ('2' == judge){	// pcm:wav -> u-law:wav
        while (1 == fread(&pcm_val, sizeof(short), 1, fpin)){
            c = linear2ulaw(pcm_val);
            fputc(c, fpout);
        }
    }
    else if ('3' == judge){	// u-law:wav -> pcm:wav
        while (1 == fread(&c, sizeof(char), 1, fpin)){
            pcm_val = ulaw2linear(c);
            fwrite(&pcm_val, sizeof(short), 1, fpout);
        }
    }
    printf("OK!\n");
END:
    fclose(fpin);
    fclose(fpout);
}
