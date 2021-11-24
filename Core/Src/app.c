
#include "main.h"
#include "string.h"

#define RXBUFFERSIZE  256     //最大接收字节数
char RxBuffer2[RXBUFFERSIZE];   //接收数据
char RxBuffer3[RXBUFFERSIZE];   //接收数据
uint8_t aRxBuffer1;            //接收中断缓冲
uint8_t aRxBuffer2;            //接收中断缓冲
uint8_t aRxBuffer3;            //接收中断缓冲
uint8_t Uart2_Rx_Cnt = 0;        //接收缓冲计数
uint8_t Uart3_Rx_Cnt = 0;        //接收缓冲计数
extern UART_HandleTypeDef huart3;
extern UART_HandleTypeDef huart2;
extern UART_HandleTypeDef huart1; //交换机串口

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
    /* Prevent unused argument(s) compilation warning */
    UNUSED(huart);
    /* NOTE: This function Should not be modified, when the callback is needed,
             the HAL_UART_TxCpltCallback could be implemented in the user file
     */


    if (huart == &huart1) { //交换机返回
        HAL_UART_Transmit(&huart2, &aRxBuffer1, 1, 0xFFFF); //向网络发送数据
        while (HAL_UART_GetState(&huart2) == HAL_UART_STATE_BUSY_TX);
        HAL_UART_Receive_IT(&huart1, (uint8_t *) &aRxBuffer1, 1);   //再开启接收中断
    } else {
        if (Uart2_Rx_Cnt >= 255)  //溢出判断
        {
            Uart2_Rx_Cnt = 0;
            memset(RxBuffer2, 0x00, sizeof(RxBuffer2));
            HAL_UART_Transmit(&huart2, (uint8_t *) "数据溢出", 10, 0xFFFF);
        } else {
            RxBuffer2[Uart2_Rx_Cnt++] = aRxBuffer2;   //接收数据转存

            if ((RxBuffer2[Uart2_Rx_Cnt - 1] == 0x0A) && (RxBuffer2[Uart2_Rx_Cnt - 2] == 0x0D)) //判断结束
            {
                if (strcmp(RxBuffer2, "RELAY=0\r\n") == 0) {
                    HAL_UART_Transmit(&huart2, (uint8_t *) &"关闭开关\r\n", 14, 0xFFFF);
                    HAL_GPIO_WritePin(RELAY_GPIO_Port, RELAY_Pin, GPIO_PIN_RESET);
                } else if (strcmp(RxBuffer2, "RELAY=1\r\n") == 0) {
                    HAL_UART_Transmit(&huart2, (uint8_t *) &"打开开关\r\n", 14, 0xFFFF);
                    HAL_GPIO_WritePin(RELAY_GPIO_Port, RELAY_Pin, GPIO_PIN_SET);
                } else {
                    uint8_t len = strlen(RxBuffer2) - 2;
                    char str[len];
                    if (strcmp(RxBuffer2, "Enter\r\n") == 0) {
                        strcpy(str,"\r\n");
                    } else {
                        strncpy(str, RxBuffer2, len);
                    }
//                    HAL_UART_Transmit(&huart2, (uint8_t *) &"收到命令,", 13, 0xFFFF);
//                    HAL_UART_Transmit(&huart2, (uint8_t *) &str, strlen(str), 0xFFFF);
                    HAL_UART_Transmit(&huart1, (uint8_t *) &str, strlen(str), 0xFFFF);//发送命令到交换机
                }
                while (HAL_UART_GetState(&huart2) == HAL_UART_STATE_BUSY_TX);//是否发送结束
                while (HAL_UART_GetState(&huart1) == HAL_UART_STATE_BUSY_TX);//是否发送结束
                Uart2_Rx_Cnt = 0;
                memset(RxBuffer2, 0x00, sizeof(RxBuffer2)); //清空数组
            }
        }
        HAL_UART_Receive_IT(&huart2, (uint8_t *) &aRxBuffer2, 1);   //再开启接收中
    }

}


