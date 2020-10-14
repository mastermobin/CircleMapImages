#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
#include <Windows.h>
#include <time.h>
#include <stdbool.h>
#include <math.h>
#include <new.h>
#include <pthread.h>

#define CIRCLE_COUNT 300
#define R_DEFAULT 255
#define G_DEFAULT 255
#define B_DEFAULT 255
#define SCALE 1
#define X_OFFSET 400
#define ALLOWED_DISTANCE 200

struct Target{
    long int Profit;
    int Index;
};

struct Location{
    unsigned char X,Y;
};

struct Area{
    Location MyLocation;
    float Order;
    unsigned char Radius;
};

struct Color{
    unsigned char R,G,B;
};

struct Circle{
    unsigned char Radius;
    Location MyLocation;
    Color MyColor;
    int Alpha;
};

int Width , Height, CountForBM = 1;
unsigned long int LastFit;
long int Differences[CIRCLE_COUNT + 1] = {0};
Color **ImageRGB;
Color ***TempImageRGB;
Circle Models[CIRCLE_COUNT];
//int DifferenceMap[150][150] = {0};
HDC MyDC;

void *threadFunc(void *args)
{
    char Ch;
    while(Ch = getch())
    {
        if(Ch == 's')
        {
            FILE* CMPFile = fopen("Output.cmp", "wb");
            fwrite(&Width,sizeof(int),1,CMPFile);
            fwrite(&Height,sizeof(int),1,CMPFile);
            fwrite(Models,sizeof(Circle),CIRCLE_COUNT,CMPFile);
            fclose(CMPFile);
        }
        else if(Ch == 'e')
            exit(0);
    }
}

void goxy(int X, int Y)
{
    HANDLE Screen;
    Screen = GetStdHandle(STD_OUTPUT_HANDLE);
    COORD Position={X, Y};

    SetConsoleCursorPosition(Screen, Position);
}

void Draw(int Index)
{
    for(int j=0;j<Height;j++){
        for(int i = 0;i<Width;i++){
            for(int m = 0;m<SCALE;m++){
                for(int n = 0;n < SCALE;n++){
                        if(Index == 0)
                            SetPixel(MyDC,X_OFFSET + SCALE*i + m,SCALE*(Height - 1) - SCALE*j - n,RGB(ImageRGB[i][j].R,ImageRGB[i][j].G,ImageRGB[i][j].B));
                        else
                            SetPixel(MyDC,X_OFFSET + SCALE*i + m,SCALE*(Height - 1) - SCALE*j - n,RGB(TempImageRGB[i][j][Index].R,TempImageRGB[i][j][Index].G,TempImageRGB[i][j][Index].B));
                }
            }
        }
    }
}

double Distant(Location FirstPoint,Location SecondPoint){
    return(sqrt((FirstPoint.X - SecondPoint.X) * (FirstPoint.X - SecondPoint.X) + ((FirstPoint.Y - SecondPoint.Y) * (FirstPoint.Y - SecondPoint.Y))));
}

unsigned long int Fitness(int Index){
    unsigned long int Penalty = 0;
    unsigned long int Penalties[CIRCLE_COUNT + 1];
    for(int k = Index;k<=CIRCLE_COUNT;k++)
    {
        Penalty = 0;
        for(int i = 0;i<Width;i++)
            for(int j = 0;j<Height;j++)
            {
                int Differ =  (ImageRGB[i][j].R - TempImageRGB[i][j][k].R);
                Differ += (ImageRGB[i][j].G - TempImageRGB[i][j][k].G);
                Differ += (ImageRGB[i][j].B - TempImageRGB[i][j][k].B);
                //if(k == 300) DifferenceMap[i][j] = Differ * Differ;
                Penalty += Differ * Differ;

            }
        Penalties[k] = Penalty;
    }
    for(int i = 1;i<CIRCLE_COUNT;i++)
        Differences[i-1] = Penalties[i-1]-Penalties[i];
    return Penalty;
}

void CircleToRGB(int Index){
    Location CurrentLocation;
    for(int k = 0;k<CIRCLE_COUNT+1;k++)
            for(int i = 0;i<Width;i++)
                for(int j = 0;j<Height;j++){
                    TempImageRGB[i][j][k].R = R_DEFAULT;
                    TempImageRGB[i][j][k].G = G_DEFAULT;
                    TempImageRGB[i][j][k].B = B_DEFAULT;
            }
    Index = 1;
    for(int i = Index;i<=CIRCLE_COUNT;i++){
        int StartX = Models[i-1].MyLocation.X - Models[i-1].Radius;
        int EndX = Models[i-1].MyLocation.X + Models[i-1].Radius;
        int StartY = Models[i-1].MyLocation.Y - Models[i-1].Radius;
        int EndY = Models[i-1].MyLocation.Y + Models[i-1].Radius;
        if(StartX<0) StartX = 0;
        if(EndX>Width) EndX = Width;
        if(StartY<0) StartY = 0;
        if(EndY>Height) EndY = Height;
        for(CurrentLocation.X = StartX;CurrentLocation.X<EndX;CurrentLocation.X++)
            for(CurrentLocation.Y = StartY;CurrentLocation.Y<EndY;CurrentLocation.Y++)
            {
                double Distance = Distant(Models[i - 1].MyLocation,CurrentLocation);
                if(Distance <= Models[i-1].Radius - 1)
                {
                    Color LastColor = TempImageRGB[CurrentLocation.X][CurrentLocation.Y][i - 1];
                    Color NewColor;
                    NewColor.R = (((100 - Models[i-1].Alpha)*LastColor.R) + (Models[i-1].Alpha*Models[i-1].MyColor.R))/100;
                    NewColor.G = (((100 - Models[i-1].Alpha)*LastColor.G) + (Models[i-1].Alpha*Models[i-1].MyColor.G))/100;
                    NewColor.B = (((100 - Models[i-1].Alpha)*LastColor.B) + (Models[i-1].Alpha*Models[i-1].MyColor.B))/100;
                    for(int j = i;j <= CIRCLE_COUNT;j++)
                        TempImageRGB[CurrentLocation.X][CurrentLocation.Y][j] = NewColor;
                }
                else if(Distance < Models[i-1].Radius)
                {
                    float NewAlpha = Models[i-1].Alpha*(Models[i-1].Radius-Distance);
                    Color LastColor = TempImageRGB[CurrentLocation.X][CurrentLocation.Y][i-1];
                    Color NewColor;
                    NewColor.R = (((100 - NewAlpha)*LastColor.R) + (NewAlpha*Models[i-1].MyColor.R))/100;
                    NewColor.G = (((100 - NewAlpha)*LastColor.G) + (NewAlpha*Models[i-1].MyColor.G))/100;
                    NewColor.B = (((100 - NewAlpha)*LastColor.B) + (NewAlpha*Models[i-1].MyColor.B))/100;
                    for(int j = i;j <= CIRCLE_COUNT;j++)
                        TempImageRGB[CurrentLocation.X][CurrentLocation.Y][j] = NewColor;
                }
            }
    }
}

void InitializeArray(){
    srand(time(NULL));
    int i;
    for(i = 0;i < CIRCLE_COUNT - 60 ;i++)
    {
        unsigned char X,Y;
        X = (Width / 15) * (i % 15);
        Y = (Height / 15) * (i/ 15);
        Models[i].MyLocation.X = X;
        Models[i].MyLocation.Y = Y;
        Models[i].Alpha = 100;
        Models[i].Radius = ((Height > Width) ? Width : Height)/30;
        Models[i].MyColor = ImageRGB[X][Y];
    }
    for(;i<CIRCLE_COUNT ;i++)
    {
        Models[i].MyLocation.X = rand() % Width;
        Models[i].MyLocation.Y = rand() % Height;
        Models[i].Alpha = ((rand() % 40)+61);
        Models[i].MyColor = ImageRGB[Models[i].MyLocation.X][Models[i].MyLocation.Y];
        Models[i].Radius = rand() % (((Height > Width) ? Width : Height)/5);
    }
}

void Mutation(){
    srand(time(NULL));
    Target Targets[10];
    Circle LastCircle;
    for(int i = 1;i<10;i++)
        Targets[i].Profit = 10000000;
    Targets[0].Index = 0;
    Targets[0].Profit = Differences[0];
    int Counter = 1;
    for(int i = 1;i<CIRCLE_COUNT;i++)
    {
        for(int j = 0;j<10;j++)
        {
            if(Differences[i] < Targets[j].Profit)
            {
                for(int k = Counter;k>j;k--)
                {
                    Targets[k+1] = Targets[k];
                }
                if(Counter != 9) Counter++;
                Targets[j].Profit = Differences[i];
                Targets[j].Index = i;
                break;
            }
        }
    }
    for(int i = 0;i<10;i++)
    {
        int CurrentIndex = Targets[i].Index;
        LastCircle = Models[CurrentIndex];

        Models[CurrentIndex].MyLocation.X = rand() % Width;
        Models[CurrentIndex].MyLocation.Y = rand() % Height;
        Models[CurrentIndex].Alpha = ((rand() % 40)+61);
        Models[CurrentIndex].MyColor = ImageRGB[Models[CurrentIndex].MyLocation.X][Models[CurrentIndex].MyLocation.Y];
        Models[CurrentIndex].Radius = rand() % (((Height > Width) ? Width : Height)/5);
        if(CurrentIndex != 0)
            CircleToRGB(CurrentIndex);
        else
            CircleToRGB(1);
        unsigned long int Fit = Fitness(CurrentIndex);
        if(LastFit > Fit )
        {
            goxy(0,0);
            printf("                                                         \n                                                    \n                                            ");
            goxy(0,0);
            printf("%5d'st Score : %li\nClick (s) To Save\nClick (e) To Exit",CountForBM,Fit);
            LastFit = Fit;
        }
        else
        {
            Models[CurrentIndex] = LastCircle;
            if(CurrentIndex != 0)
                CircleToRGB(CurrentIndex);
            else
                CircleToRGB(1);
            Fitness(CurrentIndex);
        }
    }

}

void SmallMutation(){
    srand(time(NULL));
    Target Targets[10];
    Circle LastCircle;
    for(int i = 1;i<10;i++)
        Targets[i].Profit = -10000000;
    Targets[0].Index = 0;
    Targets[0].Profit = Differences[0];
    int Counter = 1;
    for(int i = 1;i<CIRCLE_COUNT;i++)
    {
        for(int j = 0;j<10;j++)
        {
            if(Differences[i] > Targets[j].Profit)
            {
                for(int k = Counter;k>j;k--)
                {
                    Targets[k+1] = Targets[k];
                }
                if(Counter != 9) Counter++;
                Targets[j].Profit = Differences[i];
                Targets[j].Index = i;
                break;
            }
        }
    }
    for(int i = 0;i<10;i++)
    {
        int CurrentIndex = Targets[i].Index;
        LastCircle = Models[CurrentIndex];
        char DeltaX = (rand()%11)-5;
        char DeltaY = (rand()%11)-5;
        char DeltaAlpha = ((rand()%11)-5)/10.0;
        char DeltaRadius = (rand()%11)-5;
        unsigned char NewX = Models[CurrentIndex].MyLocation.X + DeltaX;
        if(NewX > Width - 1) NewX = Width - 1;
        if(NewX<0) NewX = 0;
        unsigned char NewY = Models[CurrentIndex].MyLocation.Y + DeltaY;
        if(NewY > Height - 1) NewY = Height - 1;
        if(NewY<0) NewY = 0;
        unsigned char NewAlpha = Models[CurrentIndex].Alpha + DeltaAlpha;
        if(NewAlpha > 100) NewAlpha = NewAlpha;
        if(NewAlpha<0) NewAlpha = 0;
        unsigned char NewRadius = Models[CurrentIndex].Radius + DeltaRadius;
        if(NewRadius < 1) NewRadius = 1;
        Models[CurrentIndex].MyLocation.X = NewX;
        Models[CurrentIndex].MyLocation.Y = NewY;
        Models[CurrentIndex].Alpha = NewAlpha;
        Models[CurrentIndex].MyColor = ImageRGB[Models[CurrentIndex].MyLocation.X][Models[CurrentIndex].MyLocation.Y];
        Models[CurrentIndex].Radius = NewRadius;
        if(CurrentIndex != 0)
            CircleToRGB(CurrentIndex);
        else
            CircleToRGB(1);
        unsigned long int Fit = Fitness(CurrentIndex);
        if(LastFit > Fit )
        {
            goxy(0,0);
            printf("                                                         \n                                                    \n                                            ");
            goxy(0,0);
            printf("%5d'st Score : %li\nClick (s) To Save\nClick (e) To Exit",CountForBM,Fit);
            LastFit = Fit;
        }
        else
        {
            Models[CurrentIndex] = LastCircle;
            if(CurrentIndex != 0)
                CircleToRGB(CurrentIndex);
            else
                CircleToRGB(1);
            Fitness(CurrentIndex);
        }
    }

}

int main(){

    HWND MyConsole = GetConsoleWindow();
    MyDC = GetDC(MyConsole);
    ShowWindow(MyConsole,SW_SHOWMAXIMIZED);
    printf("Welcome To My Project !!! \nWrite First Word Of An Option To Start\n1 - (R)ead CMP File And Show That\n2 - (C)onvert BMP To CMP\n");
    char C;
    while(C = getch())
    {
        if(C == 'C' || C == 'c')
        {
            goxy(0,0);
            printf("                                                         \n                                                    \n                                            \n                                                                         ");
            goxy(0,0);
            SetConsoleTitle("Image Processing : Get Input");
            char FilePath[500];
            printf("Insert Your Bitmap Photo Address : ");
            gets(FilePath);
            int i, Padding = 0;
            FILE* MyFile = fopen(FilePath, "rb");
            if(MyFile != NULL){
                unsigned char info[54];
                fread(info, sizeof(unsigned char), 54, MyFile);

                Width = *(int*)&info[18];
                Height = *(int*)&info[22];

                Padding = 4 - ((Width * 3) % 4);//Calculate Gap At Every Row
                if(Padding == 4){
                    Padding = 0;
                }

                int Size = 3 * Width * Height;
                unsigned char Data[Size];
                fread(Data, sizeof(unsigned char), Size + (Padding * Height) , MyFile);
                fclose(MyFile);

                ImageRGB = (Color**) malloc(Width * sizeof(Color*));
                for (int i = 0; i < Width; i++) {
                  ImageRGB[i] = (Color*) malloc(Height * sizeof(Color));
                }

                TempImageRGB = (Color***) malloc(Width * sizeof(Color**));
                for(int i = 0;i<Width;i++){
                    TempImageRGB[i] = (Color**) malloc(Height * sizeof(Color*));
                    for(int j = 0;j<Height;j++){
                        TempImageRGB[i][j] = (Color*) malloc((CIRCLE_COUNT + 1) * sizeof(Color));
                    }
                }

                for(int j = 0;j < Height;j++){
                    for(int i = 0;i < Width;i++){
                        for(int k = 0;k<3;k++){
                            if(k == 1)
                                ImageRGB[i][j].G = Data[(((j * Width) + i)*3) + (Padding * j) + k];//Convert BGR To RGB
                            else if(k == 2)
                                ImageRGB[i][j].R = Data[(((j * Width) + i)*3) + (Padding * j) + k];//Convert BGR To RGB
                            else
                                ImageRGB[i][j].B = Data[(((j * Width) + i)*3) + (Padding * j) + k];//Convert BGR To RGB
                        }
                    }
                }

                SetConsoleTitle("Image Processing : Start");

                for(int k = 0;k<CIRCLE_COUNT+1;k++)
                    for(int i = 0;i<Width;i++)
                        for(int j = 0;j<Height;j++){
                            TempImageRGB[i][j][k].R = R_DEFAULT;
                            TempImageRGB[i][j][k].G = G_DEFAULT;
                            TempImageRGB[i][j][k].B = B_DEFAULT;
                    }
                pthread_t pth;
                pthread_create(&pth,NULL,threadFunc,NULL);
                SetConsoleTitle("Image Processing : Initialize Random");
                InitializeArray();
                SetConsoleTitle("Image Processing : Convert To RGB ");
                CircleToRGB(1);
                LastFit = Fitness(0);
                for(;;)
                {
                        Mutation();
                        if(LastFit < 50000000)
                        SmallMutation();
                        Draw(300);
                        CountForBM++;
                }
            }
        }
        else if(C == 'R' || C == 'r')
        {
            goxy(0,0);
            printf("                                                         \n                                                    \n                                            \n                                             ");
            goxy(0,0);
            char FilePath[500];
            printf("Insert Your Bitmap Photo Address : ");
            gets(FilePath);
            FILE* CMPFile = fopen(FilePath,"rb");
            if(CMPFile != NULL)
            {
                fread(&Width,sizeof(int),1,CMPFile);
                fread(&Height,sizeof(int),1,CMPFile);
                ImageRGB = (Color**) malloc(Width * sizeof(Color*));
                for (int i = 0; i < Width; i++) {
                  ImageRGB[i] = (Color*) malloc(Height * sizeof(Color));
                }

                TempImageRGB = (Color***) malloc(Width * sizeof(Color**));
                for(int i = 0;i<Width;i++){
                    TempImageRGB[i] = (Color**) malloc(Height * sizeof(Color*));
                    for(int j = 0;j<Height;j++){
                        TempImageRGB[i][j] = (Color*) malloc((CIRCLE_COUNT + 1) * sizeof(Color));
                    }
                }
                fread(Models,sizeof(Circle),CIRCLE_COUNT,CMPFile);
                CircleToRGB(1);
                Draw(300);
            }else{
                exit(0);
            }
        }
    }

    getch();
}
