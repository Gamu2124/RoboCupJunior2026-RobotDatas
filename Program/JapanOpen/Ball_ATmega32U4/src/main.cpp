#include <Arduino.h>
#include <math.h>

const uint8_t ARR_LENGTH = 5;
int Ball_num[ARR_LENGTH];
int arr_center = 2;
uint8_t first = 0;
uint8_t Ball_digital[16];
uint8_t Ball_count[16];
uint8_t Ball_best_val = 0;
uint8_t Ball_sum = 0;
uint8_t Ball_best_num = 0;

byte sendBuf_byte[8];

void get_Ball();
void print();

float pretime = 0;

void setup(){
    /* ===== 入力ピン設定 ===== */
    DDRC &= ~((1 << PC7) | (1 << PC6));               // PC7, PC6
    DDRB &= ~((1 << PB6) | (1 << PB5) | (1 << PB4) |  // PB6..PB0
              (1 << PB3) | (1 << PB2) | (1 << PB1) | (1 << PB0));
    DDRD &= ~((1 << PD7) | (1 << PD6) | (1 << PD4)); // PD7, PD6, PD4
    DDRF &= ~((1 << PF0) | (1 << PF1) | (1 << PF4) | (1 << PF5)); // PF系

    Serial1.begin(115200);
    Serial.begin(9600);

    while (!Serial1) {}
    delay(1500);
}

void loop(){
    get_Ball();

    sendBuf_byte[0] = 0xFF;

    if(Ball_sum == 0){
        for(int i = 1; i <= 6; i++){
            sendBuf_byte[i] = 0x80;
        }
    } else {
        sendBuf_byte[1] = Ball_best_num;
        sendBuf_byte[2] = Ball_count[Ball_num[0]];
        sendBuf_byte[3] = Ball_count[Ball_num[1]];
        sendBuf_byte[4] = Ball_count[Ball_num[2]];
        sendBuf_byte[5] = Ball_count[Ball_num[3]];
        sendBuf_byte[6] = Ball_count[Ball_num[4]];
    }

    sendBuf_byte[7] = 0;
    Serial1.write(sendBuf_byte, 8);
}

void get_Ball()
{
    memset(Ball_count, 0, sizeof(Ball_count));
    memset(Ball_num, 0, sizeof(Ball_num));

    Ball_best_val = 0;
    Ball_sum = 0;
    Ball_best_num = 0;

    for(int j = 0; j < 80; j++){
        Ball_digital[0]  = PINC & _BV(7); // PC7
        Ball_digital[1]  = PINC & _BV(6); // PC6
        Ball_digital[2]  = PINB & _BV(6); // PB6
        Ball_digital[3]  = PINB & _BV(5); // PB5
        Ball_digital[4]  = PINB & _BV(4); // PB4
        Ball_digital[5]  = PIND & _BV(7); // PD7
        Ball_digital[6]  = PIND & _BV(6); // PD6
        Ball_digital[7]  = PIND & _BV(4); // PD4
        Ball_digital[8]  = PINB & _BV(3); // PB3
        Ball_digital[9]  = PINB & _BV(2); // PB2
        Ball_digital[10] = PINB & _BV(1); // PB1
        Ball_digital[11] = PINB & _BV(0); // PB0
        Ball_digital[12] = PINF & _BV(0); // PF0
        Ball_digital[13] = PINF & _BV(1); // PF1
        Ball_digital[14] = PINF & _BV(4); // PF4
        Ball_digital[15] = PINF & _BV(5); // PF5

        for(int i = 0; i < 16; i++){
            if(Ball_digital[i] == 0){
                Ball_count[i]++;
                Ball_sum++;
            }
        }
    }

    for(int i = 0; i < 16; i++){
        if(Ball_best_val < Ball_count[i]){
            Ball_best_val = Ball_count[i];
            Ball_best_num = i;
        }
    }

    for(int i = 0; i < ARR_LENGTH; i++){
        Ball_num[i] = Ball_best_num + (i - arr_center);
        if(Ball_num[i] < 0)  Ball_num[i] += 16;
        if(Ball_num[i] > 15) Ball_num[i] -= 16;
    }
}

void print(){
    Serial.print("best : ");
    Serial.println(Ball_best_num);
}
