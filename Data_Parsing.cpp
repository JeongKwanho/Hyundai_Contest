#include "mbed.h"
#include <cmath>
#include <cstdint>
#include <utility>
#include "math.h"

#define DATA_UPDATE_RATE 80
#define MAIN_LOOP 60
#define PRINT_INTERVAL 200
#define SBUF_SIZE 400 // 시리얼 버퍼 크기

//--------------------------//
RawSerial pc(USBTX, USBRX, 115200);

Thread DataRead_thread(osPriorityHigh); 
Thread Print_thread(osPriorityNormal); 

volatile float PC_data[100]; // 데이터 저장 몇개나 받을지
bool gotPacket = false;

void DataRead_loop();
void PcParser();
void PRINT_thread_loop();
int my_strcmp(char* str1, char* str2);

float A_motor;
float B_motor;
float C_motor;
float D_motor;


int main()
{
    uint32_t Now,Work;

    osThreadSetPriority(osThreadGetId(),osPriorityRealtime7);

    pc.attach(&PcParser); // uart 인터럽트 콜백
    DataRead_thread.start(&DataRead_loop); // 파싱 데이터 읽어오기 쓰레드
    Print_thread.start(&PRINT_thread_loop);

    while(1)
    {

        Now =rtos::Kernel::get_ms_count();

        pc.printf("%f\n", D_motor);
        Work =rtos::Kernel::get_ms_count();
        ThisThread::sleep_until(rtos::Kernel::get_ms_count()+(MAIN_LOOP-(Work-Now)));
    }
}

void DataRead_loop()
{
    uint32_t Now_M,Work_M;

    pc.printf("DataRead_start\n");

    while(1)
    {
        Now_M=rtos::Kernel::get_ms_count();
        // pc.printf("%d %f p\n",API_weather,PC_data[0]);

        if(gotPacket)
        {
                
                gotPacket = false;

                D_motor = PC_data[0];
                B_motor = PC_data[2];
                C_motor = PC_data[1];
                A_motor = PC_data[3];
        }

        Work_M=rtos::Kernel::get_ms_count();
        ThisThread::sleep_until(rtos::Kernel::get_ms_count()+(DATA_UPDATE_RATE-(Work_M-Now_M)));
    }
}

void PcParser() //Data 시리얼 파싱
{
    static char pc_serialInBuffer[SBUF_SIZE];//시리얼 버퍼 배열 만들기
    static int pc_data_cnt=0,pc_buff_cnt=0;

    if(pc.readable())
    { // 통신 가능하면
        char byteIn_pc= pc.getc(); // 시리얼 통신 문자
        // pc.printf("%c",byteIn_pc);
        if (byteIn_pc=='*') // 시작문자 -> 초기화
        {
            pc_buff_cnt=0;
            pc_data_cnt=0;
        }

        else if(byteIn_pc==',') // weather(첫번쨰) 경우 외엔 모두 float 데이터로 변환해서 저장
        {
            pc_serialInBuffer[pc_buff_cnt]='\0';
            PC_data[pc_data_cnt++]=atof(pc_serialInBuffer);
            pc_buff_cnt=0;
        }

        else if(byteIn_pc=='\n'){ //끝문자
                // pc.printf("hi\n");
                pc_serialInBuffer[pc_buff_cnt]='\0';
                PC_data[pc_data_cnt]=atof(pc_serialInBuffer);

                gotPacket= true;
        }

        else
        { // 데이터 들어오는중
                pc_serialInBuffer[pc_buff_cnt++]=byteIn_pc;
        }
        
        if(pc_buff_cnt>=SBUF_SIZE) pc_buff_cnt=0; // 버퍼 넘치기 방지
    }
}

void PRINT_thread_loop()// 디버깅 PRINT
{ 
    uint32_t Now_P,Work_P;

    pc.printf("Print_start\n");

    while(true)
    {
        Now_P=rtos::Kernel::get_ms_count();

        //printf("DEBUG: %f\n", API_humidity);
        //pc.printf("%f, %f, %d, %f, %f, %f, %f, %f\n", sensor_temp, sensor_humid, API_weather, API_humidity, API_temperature, API_wind_speed, input_temp, input_hum);
        //pc.printf("%d %f p\n",API_weather,PC_data[0]);

        Work_P=rtos::Kernel::get_ms_count();
        ThisThread::sleep_until(rtos::Kernel::get_ms_count()+(PRINT_INTERVAL-(Work_P-Now_P)));
    }
}

int my_strcmp(char* str1, char* str2)
{
   int i = 0;
   // 한쪽 문자열이 끝날때까지 비교
   while (str1[i] != '\0' || str2[i] != '\0') {
   
      // 문자열 같으면 계속
      if (str1[i] == str2[i])
      {
           i++;   
             continue;
        }

      // 앞에 문자쪽 1
      else if (str1[i] > str2[i])
         return 1;

      // 뒤에 문자쪽 -1
      else if (str1[i] < str2[i])
         return -1;
      i++;
   }

   // 어느 한쪽 문자열이 끝났고 i - 1 까지 모두 같음
   if (str1[i] == str2[i])      // str1[i] == str2[i] == '\0' 문자열 끝남
       return 0;
   else if (str1[i] != '\0')   // str1에 글자가 남아있으면 1 리턴
      return 1;
   else return -1;            // str2에 글자가 남아있으면 -1 리턴
}
