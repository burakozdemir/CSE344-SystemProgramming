#include <stdio.h>
#include <stdlib.h>
#include <tgmath.h>
#include <stdint.h>

#define MMORDER 77 // in decimal
#define IIORDER 73 // in decimal

/**
 * byteın degerini integer olarak alır ve 8 bitlik char arrayı dondurur.
 */
///https://www.quora.com/Is-there-a-function-in-C-that-converts-an-integer-into-bits
unsigned char *return_byte (int source) {
    int nbits = sizeof(source) * 2;
    unsigned char *s = (unsigned char*)malloc(nbits+1);  // +1 for '\0' terminator
    s[nbits] = '\0';
    // forcing evaluation as an unsigned value prevents complications
    // with negative numbers at the left-most bit
    unsigned int u = *(unsigned int*)&source;
    int i;
    unsigned int mask = 1 << (nbits-1); // fill in values right-to-left
    for (i = 0; i < nbits; i++, mask >>= 1)
        s[i] = ((u & mask) != 0) + '0';

    return s;
}
/**
 * Motorola byte olan 2 bytelık datayı Intel tarzına donusturur
 */
///https://stackoverflow.com/questions/2182002/convert-big-endian-to-little-endian-in-c-without-using-provided-func
uint16_t convert16MtoI(uint16_t val){
    uint16_t res;
    res = (val>>8) | (val<<8);
    return res;
}
/**
 * Motorola byte olan 4 bytelık datayı Intel tarzına donusturur
 */
///https://stackoverflow.com/questions/2182002/convert-big-endian-to-little-endian-in-c-without-using-provided-func
uint32_t convert32MtoI(uint32_t val){
    uint32_t res;
    res=((val>>24)&0xff) | // move byte 3 to byte 0
        ((val<<8)&0xff0000) | // move byte 1 to byte 2
        ((val>>8)&0xff00) | // move byte 2 to byte 1
        ((val<<24)&0xff000000); // byte 0 to byte 3
    return res;
}

/**
 * Ekrana boardu basar
 * @param filename
 * @param headerOff
 * @param width
 * @param height
 * @param stripbyteCount
 * @param bitsperSample
 * @param photometric
 */
int printBoard(const char *filename,int headerOff,int stripOFF,int width,int height,int stripbyteCount,int bitsperSample,
                int photometric){
    printf("Width : %d pixels\n",width);
    printf("Height : %d pixels\n",height);

    FILE *fp=fopen (filename, "rb");

    if (!fp) {
        fprintf(stderr, "error: file open failed '%s'.\n", filename);
        return -1;
    }

    unsigned char *dataOnebit;
    uint8_t *data8bit;
    uint16_t *data16bit;

    data16bit=(uint16_t*) malloc(abs(headerOff-stripOFF)*sizeof(uint16_t));
    data8bit = (uint8_t *) malloc(abs(headerOff - stripOFF) * sizeof(uint8_t));

    dataOnebit=(unsigned char*)malloc(abs((headerOff-stripOFF)*8)*sizeof(unsigned char));


    ///tek bytelık data
    int dataIndis = 0;
    uint8_t dataTemp;
    fseek(fp, stripOFF, SEEK_SET);
    for (int k = stripOFF; k < headerOff; ++k) {
        fread(&dataTemp, sizeof dataTemp, 1, fp);
        data8bit[dataIndis] = dataTemp;
        dataIndis++;
    }

    ///tek bitlik data
    int bitDataIndis=0;
    unsigned char *temp;
    for (int j = 0; j < headerOff-8; ++j) {
        temp=return_byte(data8bit[j]);
        for (int i = 0; i < 8; ++i) {
            dataOnebit[bitDataIndis]=temp[i];
            bitDataIndis++;
        }

        free(temp);
    }

    if(bitsperSample==8){
        int byteIndis=0;
        for (int k = 0; k < height; ++k) {
            for (int i = 0; i < width; ++i) {
                if(data8bit[byteIndis]==0){
                    printf("%d",!photometric);
                }
                else
                    printf("%d",photometric);
                byteIndis++;
            }
            printf("\n");
        }
    }
    else if(bitsperSample==1){
        int reset=stripbyteCount/height;
        reset=8*reset;
        bitDataIndis=0;
        for (int k = 0; k < height; ++k) {
            for (int i = 0; i < width; ++i) {
                //printf("%c",dataOnebit[bitDataIndis]);
                if((dataOnebit[bitDataIndis]-48)==0){
                    printf("%d",!photometric);
                }
                else
                    printf("%d",photometric);
                
                bitDataIndis++;
            }
            printf("\n");
            bitDataIndis=bitDataIndis+(reset-width);
        }
    }
    else
        printf("Unsoppurted bitspersample");

    fclose(fp);
    free(dataOnebit);
    free(data16bit);
    free(data8bit);
    return 1;
}

/**
 * Dosyadaki tagler ve header a göre gerekli bilgileri ceker be printBoard fonksyonuna gönderir.
 * @param filename
 * @param ORDER
 * @return
 */
int findPixelsInPhoto(const char *filename, int ORDER) {

    //header bilgileri
    uint16_t order, versiyon, temp;
    uint32_t headerOff;

    //functtiona gonderılecek tagın bılgılerı
    uint16_t tag,type,datafield=0;
    uint32_t dataCount=0;

    int stripOffset=0;
    int counterForTags = 0;//tag sayisi icin indis
    int numberOftag;//dosyadakı tag sayısı
    int counter = 0;//file icin imlec(file pointer icin)
    int width = 0,//genislik
            height = 0, //yukseklik
            bitsperSample = 0, //pixel basina bit
            stripByteCount = 0;//byte sayısı serıt
    int photometric = 0;//0 whiteisZero 1 blackisZero

    FILE *fp = fopen(filename, "rb");
    FILE *tagPointer = fopen(filename, "rb");

    if (!fp || !tagPointer) {
        fprintf(stderr, "error: file open failed '%s'.\n", filename);
        return -1;
    }

    rewind(fp);

    /*header kisimlarının okunması*/
    fread(&order, sizeof order, 1, fp);
    //printf("Byte order:%02x\n", order);
    fread(&versiyon, sizeof versiyon, 1, fp);
    if(ORDER==77)versiyon = convert16MtoI(versiyon);
    //printf("versiyon:%d\n", versiyon);
    fread(&headerOff, sizeof headerOff, 1, fp);
    if(ORDER==77)headerOff = convert32MtoI(headerOff);
    //printf("offset:%d(decimal)\n", headerOff);

    rewind(fp);

    fseek(fp, headerOff, SEEK_CUR);//imleci tag baslangıcına aldık
    fread(&temp, sizeof temp, 1, fp);//tag sayısı okundu
    if(ORDER==77)temp = convert16MtoI(temp);
    numberOftag = temp;
    //printf("tagcount: %d (decimal)\n\n", numberOftag);

    /*Counter i dosyadaki ilk tag baslangıcına aldik*/
    counter = headerOff + 2;

    while (counterForTags < numberOftag) {
        fseek(tagPointer, counter, SEEK_SET);//pointer countera ayarlandı

        ///tag
        fread(&tag, sizeof tag, 1, tagPointer);
        if(ORDER==77)tag=convert16MtoI(tag);

        ///tagtype
        fread(&type, sizeof type, 1, tagPointer);
        if(ORDER==77)type=convert16MtoI(type);//MMORDER da byte convert edilir

        ///datacount
        fread(&dataCount, sizeof dataCount, 1, tagPointer);
        if(ORDER==77)dataCount=convert32MtoI(dataCount);//MMORDER da byte convert edilir

        ///dataField
        /*3 ise word 4 ise long*/
        if(type==3) {
            if(ORDER==77){
                fread(&datafield, sizeof datafield, 1, tagPointer);
                datafield=convert16MtoI(datafield);//MMORDER da byte convert edilir
                fseek(tagPointer,2,SEEK_CUR);
            }else{
                fread(&datafield, sizeof datafield, 1, tagPointer);
                fseek(tagPointer,2,SEEK_CUR);
            }
        }
        if(type==4){
            if(ORDER==77){
                fseek(tagPointer,2,SEEK_CUR);
                fread(&datafield, sizeof datafield, 1, tagPointer);
                datafield=convert16MtoI(datafield);//MMORDER da byte convert edilir
            }else{
                fread(&datafield, sizeof datafield, 1, tagPointer);
                fseek(tagPointer,2,SEEK_CUR);
            }
        }


        counter += 12;//counter diger tagın basına atladı
        fseek(fp, counter, SEEK_SET);//pointer set
        counterForTags++;


        if (tag != 0 && tag == 256) {
            width = dataCount*datafield;
        }
        if (tag != 0 && tag == 257) {
            height = dataCount*datafield;
        }
        if (tag != 0 && tag == 258) {
            bitsperSample = dataCount*datafield;
        }
        if (tag != 0 && tag == 279) {
            stripByteCount = dataCount*datafield;
        }
        if (tag != 0 && tag == 262) {
            photometric = datafield;
        }
        if (tag != 0 && tag == 273) {
            stripOffset = dataCount*datafield;
        }

    }

    //bilgilerin ve boardun ekrana basilmasi
    printBoard(filename,headerOff,stripOffset, width, height, stripByteCount, bitsperSample, photometric);

    if (fp != stdin)
        fclose (fp);
    if (tagPointer != stdin)
        fclose (tagPointer);
    return 1;

}

/**
 * Program icin ana fonksiyon
 * @param filename
 * @return -1 or 1
 */
int parsingPhoto(const char *filename){
    FILE *file = fopen(filename, "rb");
    uint8_t order;

    if (!file) {
        fprintf(stderr, "error: file open failed '%s'.\n", filename);
        return -1;
    }

    /*Read order for ımage file*/
    fseek(file,0,SEEK_SET);
    fread(&order, sizeof order, 1, file);

    if(order==MMORDER){
        printf("Byte order: Motorola\n");
        findPixelsInPhoto(filename, MMORDER);
    }
    else if(order==IIORDER){
        printf("Byte order: INtel\n");
        findPixelsInPhoto(filename, IIORDER);
    }else {
        printf("error\n");
        return -1;
    }

    fclose(file);
    return 1;
}

/**
 * Main
 * @param argc
 * @param argv
 * @return 0 or -1
 */
int main(int argc,char **argv){
    if (argc != 2) {
        printf("Usage: ./tiffprocessor <test.tif> \n");
        return -1;
    }
    FILE *fp=fopen(argv[1],"rb");
    int size;
    if (NULL != fp) {
        fseek (fp, 0, SEEK_END);
        size = ftell(fp);

        if (0 == size) {
            printf("file is empty\n");
            fclose(fp);
            return -1;
        }
    }

    if(parsingPhoto(argv[1]))
        printf("Done");
    else
        printf("Error");
    fclose(fp);
    return 0;
}
