#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <iostream>
#include <QFileDialog>
#include <QMessageBox>
#include <fstream>
#include <QImageReader>
#include <QImageIOHandler>
#include "stdio.h"
#include "stdlib.h"
#include <QBuffer>
#include <QImage>
#include <QPixmap>
#include <QLabel>
#include <QGraphicsScene>

using namespace std;

typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef unsigned int DWORD;
typedef long LONG;

typedef struct headertag{
    WORD ByteOrder;// Four bytes, indicating the size of the bitmap file
    WORD num;// Reserved word, must be 0
    WORD Reserved;// Reserved word, must be 0
    DWORD offSet;// Offset, help you skip the file header information palette to the data
}headertag;
// Define the file header of the bitmap file
// Unspecified file type, later research
//tag means a label
typedef struct bmpinfo{
    WORD direntries;
    int Width =1 ;
    int Height= 1;
    WORD biWidth;
    WORD tifftag;
    WORD tifftype;
    DWORD rowsperstrip = 0;
    DWORD stripoffset;
    DWORD stripcount;
    DWORD stripbytecount;
    DWORD stripbyteoffset;
    DWORD count;
    DWORD offsetvalue;
    DWORD dataofftype;
    int offType;
    char* strips[];
}BITMAPINFOHEADER;// Define the header of the bitmap file
void showBmpHead(headertag pBmpHead){  // Define a function to display information, pass in the file header structure
    cout<<"Bitmapfileheader:"<<endl;
    cout<<"Image type:" << pBmpHead.ByteOrder<<endl;
    cout<<"TIF type(42):" << pBmpHead.num<<endl;
    cout<<"Offset:"<<pBmpHead.offSet<<endl;
}


vector<unsigned char> RGBs;
vector<unsigned char> RGBgray;
vector<unsigned char> RGBdither;
vector<unsigned char> RGBHDR;
vector<uchar> rgbres;
vector<uchar> rgbhdr;
vector<uchar> rgbdither;

DWORD globalstripcount;
int globalWidth = 0;
int globalHeight = 0;
int cond;
char* strfile;



void ReadBigEndian(char *strFile);
void ReadFile(char *strFile){

    FILE *fpi;
    fpi = fopen(strFile,"rb");
    if(fpi!=NULL){

       // headers
        headertag imghead;
        fread(&imghead.ByteOrder,sizeof(WORD),1,fpi);
        fread(&imghead.num,sizeof(WORD),1,fpi);
        if(imghead.num != 42)
        {
            cout<<"BIg endian"<<endl;
            ReadBigEndian(strFile);
            fclose(fpi);
            return;
        }
        //test>>=8;
        fread(&imghead.offSet,sizeof(DWORD),1,fpi);
        //showBmpHead(imghead);
        bmpinfo imginfo;
        fseek(fpi,imghead.offSet,SEEK_SET);
        fread(&imginfo.direntries,sizeof(WORD),1,fpi);
        for(int i = 0 ; i < imginfo.direntries; i++)
        {
            fread(&imginfo.tifftag,sizeof(WORD),1,fpi);

            fread(&imginfo.tifftype,sizeof(WORD),1,fpi);
            fread(&imginfo.count,sizeof(DWORD),1,fpi);
            fread(&imginfo.offsetvalue,sizeof(DWORD),1,fpi);

            if(imginfo.tifftag == 256)
            {
                imginfo.Width = imginfo.offsetvalue;
            }
            if(imginfo.tifftag == 257)
            {
                imginfo.Height = imginfo.offsetvalue;
            }
            if(imginfo.tifftag == 256)
            {
                imginfo.Width = imginfo.offsetvalue;
            }
            if(imginfo.tifftag == 273)
            {
                imginfo.stripoffset = imginfo.offsetvalue;
                imginfo.stripcount = imginfo.count;
                imginfo.offType = imginfo.tifftype;
            }
            if(imginfo.tifftag == 278)
            {
                imginfo.rowsperstrip = imginfo.offsetvalue;
            }
            if(imginfo.tifftag == 279)
            {
                imginfo.stripbyteoffset = imginfo.offsetvalue;
                imginfo.stripbytecount = imginfo.count;
            }

        }

        long StripsPerImage =  ((imginfo.Height + imginfo.rowsperstrip - 1) / imginfo.rowsperstrip);
        fseek(fpi,imginfo.stripoffset,SEEK_SET);
        long temp;
        DWORD arr[imginfo.stripcount];
        for(long j = 0; j < StripsPerImage; j++){
               if(imginfo.offType == 3){
                   fread(&temp,sizeof(WORD),1,fpi);
               }
               else{
                   fread(&temp,sizeof(DWORD),1,fpi);

               }
               arr[j] = temp;
               //cout << "Strip ["<<j<<"] = " << temp << endl;
           }
        int repeater = imginfo.Width * imginfo.rowsperstrip * 3;
        int repeaterlast = imginfo.Width * imginfo.Height % imginfo.rowsperstrip * 3;
        BYTE temp3;
        vector<unsigned char> RGBinside;
        for (DWORD i = 0; i < imginfo.stripcount;i++)
        {
            fseek(fpi,arr[i],SEEK_SET);
            if(i == imginfo.stripcount-1)
            {
                for(int j =0; j < repeaterlast;j++)
                    {
                        fread(&temp3,sizeof(BYTE),1,fpi);
                        RGBinside.emplace_back(temp3);

                    }
            }
            for(int j =0; j < repeater;j++)
                {
                    fread(&temp3,sizeof(BYTE),1,fpi);
                    RGBinside.emplace_back(temp3);

                }
        }
        RGBs = RGBinside;
        globalstripcount = RGBinside.size();
        globalWidth = imginfo.Width;
        globalHeight = imginfo.Height;

        fclose(fpi);
    }
    else
    {
        cout<<"File open error!"<<endl;
    }
}
WORD swap_WORD( WORD val )
{
    return (val << 8) | (val >> 8 );
}
DWORD swap_DWORD( DWORD val )
{
    val = ((val << 8) & 0xFF00FF00 ) | ((val >> 8) & 0xFF00FF );
    return (val << 16) | (val >> 16);
}
void ReadBigEndian(char *strFile){

    FILE *fpi;
    fpi = fopen(strFile,"rb");
    if(fpi!=NULL){

       // headers
        headertag imghead;
        fread(&imghead.ByteOrder,sizeof(WORD),1,fpi);
        imghead.ByteOrder = swap_WORD(imghead.ByteOrder);
        fread(&imghead.num,sizeof(WORD),1,fpi);
        imghead.num= swap_WORD(imghead.num);
        fread(&imghead.offSet,sizeof(DWORD),1,fpi);
        imghead.offSet=swap_DWORD(imghead.offSet);
        //showBmpHead(imghead);
        bmpinfo imginfo;
        fseek(fpi,imghead.offSet,SEEK_SET);
        fread(&imginfo.direntries,sizeof(WORD),1,fpi);
        imginfo.direntries=swap_WORD(imginfo.direntries);

        for(int i = 0 ; i < imginfo.direntries; i++)
        {
            fread(&imginfo.tifftag,sizeof(WORD),1,fpi);
            imginfo.tifftag=swap_WORD(imginfo.tifftag);
            fread(&imginfo.tifftype,sizeof(WORD),1,fpi);
            imginfo.tifftype=swap_WORD(imginfo.tifftype);
            fread(&imginfo.count,sizeof(DWORD),1,fpi);
            imginfo.count=swap_DWORD(imginfo.count);
            fread(&imginfo.offsetvalue,sizeof(DWORD),1,fpi);
            if(imginfo.tifftag == 256)
            {

                imginfo.dataofftype = imginfo.tifftype;
                if(imginfo.tifftype == 3)
                {
                    imginfo.Width = swap_WORD(imginfo.offsetvalue);
                }
                else
                {
                    imginfo.Width = swap_DWORD(imginfo.offsetvalue);
                }
            }
            if(imginfo.tifftag == 257)
            {
                imginfo.Height = imginfo.offsetvalue;
                if(imginfo.tifftype == 3)
                {
                    imginfo.Height = swap_WORD(imginfo.offsetvalue);
                }
                else
                {
                    imginfo.Height = swap_DWORD(imginfo.offsetvalue);
                }
            }
            if(imginfo.tifftag == 273)
            {

                imginfo.stripcount = imginfo.count;
                imginfo.offType = imginfo.tifftype;
                if(imginfo.tifftype == 3)
                {
                    imginfo.stripoffset = swap_WORD(imginfo.offsetvalue);
                }
                else
                {
                    imginfo.stripoffset = swap_DWORD(imginfo.offsetvalue);
                }
            }
            if(imginfo.tifftag == 278)
            {

                if(imginfo.tifftype == 3)
                {
                    imginfo.rowsperstrip = swap_WORD(imginfo.offsetvalue);
                }
                else
                {
                    imginfo.rowsperstrip = swap_DWORD(imginfo.offsetvalue);
                }
            }
            if(imginfo.tifftag == 279)
            {
                imginfo.stripbyteoffset = swap_DWORD(imginfo.offsetvalue);
                imginfo.stripbytecount = imginfo.count;
            }

        }
        if(imginfo.rowsperstrip == 0)
        {
            imginfo.rowsperstrip = 4294967295;
        }
        long StripsPerImage =  ((imginfo.Height + imginfo.rowsperstrip - 1) / imginfo.rowsperstrip);

        fseek(fpi,imginfo.stripoffset,SEEK_SET);
        long temp;
        DWORD arr[imginfo.stripcount];
        for(long j = 0; j < StripsPerImage; j++){
               if(imginfo.offType == 3){
                   fread(&temp,sizeof(WORD),1,fpi);
                   temp = swap_WORD(temp);
               }
               else{
                   fread(&temp,sizeof(DWORD),1,fpi);
                   temp = swap_DWORD(temp);

               }
               arr[j] = temp;
               //cout << "Strip ["<<j<<"] = " << temp << endl;
           }
        fseek(fpi,imginfo.stripoffset,SEEK_SET);
        int repeater = imginfo.Width * imginfo.Height * 3;
        BYTE temp3;
        vector<unsigned char> RGBinside;
        if(StripsPerImage != 0)
        {
            int repeaterlast = imginfo.Width * imginfo.Height % imginfo.rowsperstrip * 3;
            for (DWORD i = 0; i < imginfo.stripcount;i++)
            {
                fseek(fpi,arr[i],SEEK_SET);
                if(i == imginfo.stripcount-1)
                {
                    for(int j =0; j < repeaterlast;j++)
                        {
                            fread(&temp3,sizeof(BYTE),1,fpi);
                            RGBinside.emplace_back(temp3);

                        }
                }
                for(int j =0; j < repeater;j++)
                    {
                        fread(&temp3,sizeof(BYTE),1,fpi);
                        RGBinside.emplace_back(temp3);

                    }
            }

        }
        else
        {
            for(int j =0; j < repeater;j++)
                {
                    fread(&temp3,sizeof(BYTE),1,fpi);
                    RGBinside.emplace_back(temp3);

                }
        }
        RGBs = RGBinside;
        globalstripcount = RGBinside.size();
        globalWidth = imginfo.Width;
        globalHeight = imginfo.Height;
        //free(imagedata);

        fclose(fpi);
    }
    else
    {
        cout<<"File open error!"<<endl;
    }
}
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_clicked()
{
    QString fname = QFileDialog::getOpenFileName(this,
                                    tr("Open Image"),
                                    QDir::currentPath()
                                    );
    QByteArray ba = fname.toLocal8Bit();
    strfile = ba.data();
    ReadFile(strfile);
    QImage sampa(RGBs.data(), globalWidth, globalHeight,globalWidth*3, QImage::Format_RGB888);
    ui->myLabel->setPixmap(QPixmap::fromImage(sampa));
    ui->disp->setText("Original Image");
    RGBgray.clear();
    RGBdither.clear();
    rgbres.clear();
    rgbhdr.clear();
    rgbdither.clear();
    cond = 0;

}





void HDR()
{
    vector<uchar> rgbgray = RGBs;
    uchar r[globalWidth][globalHeight];
    uchar g[globalWidth][globalHeight];
    uchar b[globalWidth][globalHeight];
    int k =0,l=1,m=2;
    for(int i =0;i<globalWidth;i++)
    {
        for(int j = 0; j <globalHeight;j++)
        {
            r[i][j] = RGBs[k];
            k+=3;
        }
    }
    for(int i =0;i<globalWidth;i++)
    {
        for(int j = 0; j <globalHeight;j++)
        {
            g[i][j] = RGBs[l];
            l+=3;
        }
    }
    for(int i =0;i<globalWidth;i++)
    {
        for(int j = 0; j <globalHeight;j++)
        {
            b[i][j] = RGBs[m];
            m+=3;
        }
    }
    for(int i =0;i<globalWidth;i++)
    {
        for(int j = 0; j <globalHeight;j++)
        {
            uchar R = r[i][j];
            uchar G = g[i][j];
            uchar B = b[i][j];
            int offset = 25;
            if( (int)R > 128)
            {
                R = R - offset;
            }
            else if( (int)R  < 128)
            {
                R = R + offset;

            }
            if( (int)G > 128)
            {
                G = G - offset;
            }
            else if( (int)G  < 128)
            {
                G = G + offset;
            }
            if( (int)B  > 128)
            {
                B = B - offset;
            }
            else if( (int)B  < 128)
            {
                B = B + offset;
            }
            if((int)R > 255)
            {
                R = 255;
            }
            if((int)G > 255)
            {
                G = 255;
            }
            if((int)B > 255)
            {
                B = 255;
            }
            rgbhdr.push_back(R);
            rgbhdr.push_back(G);
            rgbhdr.push_back(B);

        }
    }
//    for(DWORD i=0, j = 1, k=2 ;k<globalstripcount;i+=3,j+=3,k+=3)
//    {
//        uchar r = rgbgray[i], g = rgbgray[j], b = rgbgray[k] ;
//        uchar result = (0.299*r + 0.587*g + 0.114*b);
//        rgbhdr.push_back(result);
//        rgbhdr.push_back(result);
//        rgbhdr.push_back(result);
//    }
    RGBHDR = rgbhdr;
}
void grayscale()
{
    vector<uchar> rgbgray = RGBs;
    for(DWORD i=0, j = 1, k=2 ;k<globalstripcount;i+=3,j+=3,k+=3)
    {
        uchar r = rgbgray[i], g = rgbgray[j], b = rgbgray[k] ;
        uchar result = 0.3*r+0.59*g+0.11*b;
        rgbres.push_back(result);
        rgbres.push_back(result);
        rgbres.push_back(result);
    }
    RGBgray = rgbres;
}
void dither()
{

    int ditherMatrix[2][2] = {{0,2},{3,1}};
    int s,q;
    uchar r[globalWidth][globalHeight];
    uchar g[globalWidth][globalHeight];
    uchar b[globalWidth][globalHeight];
    int k =0,l=1,m=2;
    for(int i =0;i<globalWidth;i++)
    {
        for(int j = 0; j <globalHeight;j++)
        {
            r[i][j] = RGBgray[k];
            k+=3;
        }
    }
    for(int i =0;i<globalWidth;i++)
    {
        for(int j = 0; j <globalHeight;j++)
        {
            g[i][j] = RGBgray[l];
            l+=3;
        }
    }
    for(int i =0;i<globalWidth;i++)
    {
        for(int j = 0; j <globalHeight;j++)
        {
            b[i][j] = RGBgray[m];
            m+=3;
        }
    }
    int val = (256/((2*2+1)));
    for(int i =0;i<globalWidth;i++)
    {
        for(int j = 0; j <globalHeight;j++)
        {
          s = i%2;
          q = j%2;
            if(r[i][j]/val > ditherMatrix[s][q])
            {
                rgbdither.push_back((uchar)255);
            }
            else
            {
                rgbdither.push_back((uchar)0);
            }
            if(g[i][j]/val > ditherMatrix[s][q])
            {
                rgbdither.push_back((uchar)255);
            }
            else
            {
                rgbdither.push_back((uchar)0);
            }
            if(b[i][j]/val > ditherMatrix[s][q])
            {
                rgbdither.push_back((uchar)255);
            }
            else
            {
                rgbdither.push_back((uchar)0);
            }
        }
    }
//    uchar pixel;
//    for(int i =0 ;i < globalWidth;i++)
//    {
//        for(int j=0;j<globalHeight;j++)
//        {
//            s = i%2;
//            q = j%2;
//            int val = (256/((2*2+1)));
//            pixel = RGBgray[(j * globalWidth + i) * 3];
//            if((pixel) / val > ditherMatrix[s][q])
//            {
//                rgbdither.push_back((uchar)0);
//                rgbdither.push_back((uchar)0);
//                rgbdither.push_back((uchar)0);

//            }
//            else
//            {
//                rgbdither.push_back((uchar)255);
//                rgbdither.push_back((uchar)255);
//                rgbdither.push_back((uchar)255);
//            }
//        }
//    }
//    cout<< ((globalWidth)*(globalHeight-1)+(globalWidth-1))*3<<endl;
//    cout<<"RGBGRAY SIZE : "<<RGBgray.size()<<endl;
//    cout<<"RGBdither SIZE : "<<rgbdither.size()<<endl;
    RGBdither = rgbdither;
}
void MainWindow::on_pushButton_2_clicked()
{

     //= "C:/Users/v/Desktop/365project/autumn.tif";
//    ReadFile(strfile);
    //qDebug().nospace() << qPrintable(fname);
//    QImage sampa(RGBs.data(), globalWidth, globalHeight,globalWidth*3, QImage::Format_RGB888);
//    ui->myLabel->setPixmap(QPixmap::fromImage(sampa));

    if(cond == 0)
    {
        grayscale();
        QImage gray(RGBgray.data(), globalWidth, globalHeight,globalWidth*3, QImage::Format_RGB888);
        ui->myLabel->setPixmap(QPixmap::fromImage(gray));
        cond =1;
        ui->disp->setText("Grayscaled Image");
    }
    else if(cond == 1)
    {
        ui->disp->setText("Ordered Dither Image");
        cond =2;
        dither();
        QImage tot(RGBdither.data(), globalWidth, globalHeight,globalWidth*3, QImage::Format_RGB888);
        ui->myLabel->setPixmap(QPixmap::fromImage(tot));
        //ordered dither
    }
    else if(cond == 2)
    {
        HDR();
        QImage tou(RGBHDR.data(), globalWidth, globalHeight,globalWidth*3, QImage::Format_RGB888);
        ui->myLabel->setPixmap(QPixmap::fromImage(tou));
        ui->disp->setText("HDR Image");
        RGBgray.clear();
        RGBdither.clear();
        rgbres.clear();
        rgbdither.clear();
        cond = 55;
    }
    else if(cond ==55)
    {
        QImage sampa(RGBs.data(), globalWidth, globalHeight,globalWidth*3, QImage::Format_RGB888);
        ui->myLabel->setPixmap(QPixmap::fromImage(sampa));
        ui->disp->setText("Original Image");
        rgbhdr.clear();
        cond = 0;
    }


}
