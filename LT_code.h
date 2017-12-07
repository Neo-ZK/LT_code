#ifndef LT_CODE_H
#define LT_CODE_H
#include<iostream>
#include<fstream>
#include<sys/stat.h>
#include<string.h>
#include<math.h>
#include<time.h>
#include<stdlib.h>
using namespace std;

typedef char* (*send_callback_fun)(char*,int);

class LT_code
{
public:
	LT_code(const char* filename,double redundancy,send_callback_fun send):
	  _filename(filename),_buf(NULL),_redundancy(redundancy),_send_buf(NULL),_send(send),
		  _matrix_message(NULL),_matrix_message_row(0),
		  _H(NULL),
		  _matrix_encode(NULL),_matrix_encode_row(0),_matrix_dgree_distribution(NULL),
		  _matrix_H_decode(NULL),_matrix_message_decode(NULL){};
	  ~LT_code();
	  //process fun
	  void LT_XOR(int* arr1,int* arr2,int len,int* result);
	  void LT_XOR(char* arr1,char* arr2,int len,char* result);
	  //sender
	  //read file record as binary
	  void ReadFile();
	  int  get_filesize();
	  void encodeAndsend();
	  void init_sender();
	  int* robust_solition(int packet_num,double redundancy);
	  //receiver
	  void init_receiver(int row_max,int col_H_max,int col_message_max);
	  int receiveAndDecode(int row_max,char* buf,int& current_session_num,int& insert_index);
	  int Gussian_decode(int row_max,int col_H_max,int col_message_max);
	  int find_rank(int rank_max);
	  void WriteFile(const char* filename,int line_num);
protected:
private:
	//sender
	const char* _filename;
	char* _buf;
	double _redundancy;
	char* _send_buf;
	send_callback_fun _send;
	//message
	char** _matrix_message;
	int _matrix_message_row;
	//H
	int** _H;
	//encode
	char** _matrix_encode;
	int _matrix_encode_row;//matrix_row is the number of packages generated
	int* _matrix_dgree_distribution;
	//receiver
	int** _matrix_H_decode;
	char** _matrix_message_decode;
	//constraint of sender and receiver
	static const int _session_num_len = 4;  //the first 4 byte of the package indicate the session num
	static const int _packet_encode_num_len = 4; //the second 4 byte of the package indicate the num of of packages involved in mixing
	static const int _matrix_encode_col = 1000;// this define that the data of one package is 1KB(Type char data requires 1 bytes)
};
#endif // LT_CODE_H
