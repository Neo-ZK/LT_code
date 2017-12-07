#ifndef _LT_FUNCTION_H
#define _LT_FUNCTION_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <time.h>
//sender
#define blocksize_sessionNum 4 //This section in the block represents the session number,the size of session number in a block is 4byte
#define blocksize_packetNum 4 //This section in the block represents the number of packages involved in the mixture, with a total of 4 bytes
#define blocksize_data_Len 1000 //This section in the block represents the data, with total of 1000byte

#define REDUNDANCY 0.2

#define MAX_RETRANSMISSION_TIMES 10

//receiver
#define MAX_ROWNUM 200
#define MAX_SUCCESS_QUENUM 200

//sender
/*typedef struct send_buf
{
	int sessionNum;
	int packetNum;
	char data[1000];
	int H;
}send_buf;
*/

int encodeAndSend(char* send_data,int len,double redundancy,int linkSocket,struct sockaddr_in *destAddr,int session_num);

void initSender(char*** matrix_message,char* send_data,int send_data_len,int packetNum);

int* robustSolition(int packet_num,double redundancy);



//receiver
int receiveAndDecode(int row_max,char* buf,int* current_session_num,int* insert_index,char*** matrix_message_decode,int*** matrix_H_decode,int* success_recover_que,int* que_insert_pos);

void init_receiver(int row_max,int col_H_max,int col_message_max,char*** matrix_message_decode,int*** matrix_H_decode);

void free_receiver(int row_max,char*** matrix_message_decode,int*** matrix_H_decode);

int Gussian_decode(int row_max,int col_H_max,int col_message_max,char** matrix_message_decode,int** matrix_H_decode);

int find_rank(int rank_max,int** matrix_H_decode);

//common
void LT_XOR_int(int* arr1,int* arr2,int len,int* result);

void show_fun();

void LT_XOR_char(char* arr1,char* arr2,int len,char* result);

void row_swap_int(int** arr1,int** arr2);

void row_swap_char(char** arr1,char** arr2);


//end LT function


#endif//_LT_FUNCTION_H