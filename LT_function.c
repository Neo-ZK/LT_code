//LT function begin
#include "LT_function.h"
void show_fun()
{
	printf("OK");
}

void LT_XOR_int(int* arr1,int* arr2,int len,int* result)
{
	int i;
	for(i = 0;i < len;i++)
	{
		result[i] = arr1[i] ^ arr2[i];
	}
}

void LT_XOR_char(char* arr1,char* arr2,int len,char* result)
{
	int i;
	for(i = 0;i < len;i++)
	{
		result[i] = arr1[i] ^ arr2[i];
	}
}

void row_swap_int(int** arr1,int** arr2)
{
	int* temp;
	temp = *arr1;
	*arr1 = *arr2;
	*arr2 = temp;
}

void row_swap_char(char** arr1,char** arr2)
{
	char* temp;
	temp = *arr1;
	*arr1 = *arr2;
	*arr2 = temp;
}

/*fun:encode the message data according to the redundancy,then send to destAddr */
/*entrance:send_data:the message data
	   len:the length of data
	   redundancy:coding redundancy
	   linksocket:link socket
 	   destAddr:the sockaddr_in of destAddr
	   session_num:Custom session signs, packages have same session_num indicate that they belong to same source package                  */
/*return:No practical significance*/
int encodeAndSend(char* send_data,int len,double redundancy,int linkSocket,struct sockaddr_in *destAddr,int session_num)
{	
	char** matrix_message = NULL;
	int packetNum = (len/blocksize_data_Len) + 1;
	int* matrix_degreeDistribution = NULL;
	int matrix_encode_row = packetNum * (1 + redundancy);
	int packetTotalSend = 1;
	
	initSender(&matrix_message,send_data,len,packetNum);	
	
	
	srand(time(NULL));
	//encode and send
	{
		//inital send_buf
		char* temp_sendBuf = (char*)malloc(blocksize_sessionNum+blocksize_packetNum+blocksize_data_Len+packetNum*sizeof(int));
		int* temp_H = (int*)malloc(packetNum*sizeof(int));
		char* temp_code =(char*)malloc(blocksize_data_Len*sizeof(char));
		int col_index;
		int row_index;		
		int degree_num;
		int packet_index;
		int length = blocksize_sessionNum+blocksize_packetNum+blocksize_data_Len+packetNum*sizeof(int);
		int i;
		int byteswritten;
		matrix_degreeDistribution = robustSolition(packetNum,redundancy);		
		for(row_index = 0;row_index < matrix_encode_row;row_index++)
		{
			memset(temp_sendBuf,0,length);			
			memset(temp_H,0,packetNum*sizeof(int));
			memset(temp_code,0,blocksize_data_Len);
			degree_num = matrix_degreeDistribution[row_index];
			for(col_index = 0;col_index < degree_num;col_index++)
			{				
				packet_index = rand()%packetNum;
				if (temp_H[packet_index] == 0)
				{
					temp_H[packet_index] = 1;
					LT_XOR_char(temp_code,matrix_message[packet_index],blocksize_data_Len,temp_code);
				}
				else
				{
					col_index--;
				}
			}
			memcpy(temp_sendBuf,&session_num,blocksize_sessionNum);
			memcpy(temp_sendBuf+blocksize_sessionNum,&packetNum,blocksize_packetNum);
			memcpy(temp_sendBuf+blocksize_sessionNum+blocksize_packetNum,temp_code,blocksize_data_Len);
			memcpy(temp_sendBuf+blocksize_sessionNum+blocksize_packetNum+blocksize_data_Len,temp_H,packetNum*sizeof(int));
	
			
			byteswritten = isendto(linkSocket, temp_sendBuf, length, 0,
				(struct sockaddr *) destAddr,
				sizeof(struct sockaddr));
			if (byteswritten < 0)
			{
				if (errno == EINTR)	
				{
					continue;	
				}
				
			}
			packetTotalSend++;
		}
		free(temp_sendBuf);
		temp_sendBuf = NULL;
		free(temp_H);
		temp_H = NULL;
		free(temp_code);
		temp_code = NULL;
		if(matrix_degreeDistribution)
		{
			free(matrix_degreeDistribution);
			matrix_degreeDistribution = NULL;
		}
		for (i = 0;i < packetNum;i++)
		{
			free(matrix_message[i]);
			matrix_message[i] = NULL;
		}
		free(matrix_message);
		matrix_message = NULL;
	}
	
	return 0;
}

void initSender(char*** matrix_message,char* send_data,int send_data_len,int packetNum)
{
	int i;
	*matrix_message = (char**)malloc(packetNum*sizeof(char*));
	for (i = 0;i < packetNum;i++)
	{
		(*matrix_message)[i] = (char*)malloc(blocksize_data_Len*sizeof(char));
		memset((*matrix_message)[i],0,blocksize_data_Len*sizeof(char));
		if (i != packetNum-1)
		{
			memcpy((*matrix_message)[i],send_data+i*blocksize_data_Len,blocksize_data_Len);
		}
		else
		{
			memcpy((*matrix_message)[i],send_data+i*blocksize_data_Len,send_data_len-i*blocksize_data_Len);
		}
	}	
}

/*fun:gennerate distribution matrix according to robust soliton,return the distribution matrix In the case of REDUNDANCY of the parameter redundancy*/
int* robustSolition(int packet_num,double redundancy)
{
	int i;	
	double c = 0.05;
	double delta = 0.05; //The probability of successful decoding is guaranteed as 1- delta
	double R = c * log((double)packet_num/delta) * (double)sqrt((double)packet_num);
	int degree_max = (int)(packet_num/R);
	double* p_robust;
	int p_robust_len;
	p_robust_len = (degree_max>packet_num)?degree_max:packet_num;
	p_robust = (double*)malloc(p_robust_len*sizeof(double));
	memset(p_robust,0,p_robust_len*sizeof(double));
	p_robust[0] = (double)1/packet_num;
	//ideal solition
	for(i = 1;i < packet_num;i++)
	{
		p_robust[i] = (double)1/(i*(i+1));
	}
	//robust solition
	for(i = 0;i < degree_max;i++)
	{
		double p = R/((i+1)*packet_num);
		p_robust[i] = p_robust[i] + p;
	}
	//normalization
	{
		double sum = 0;
		double temp_min = 1;
		int min_index = 0;
		int que_num;
		int* distribution;
		int random_matrix_index;
		int j;
		int matrix_distribution_len = packet_num * (1 + redundancy);
		int* matrix_distribution = (int*)malloc(sizeof(int) * matrix_distribution_len);
		for(i = 0;i < p_robust_len;i++)
		{
			sum += p_robust[i];
		}
		
		for(i = 0;i < p_robust_len;i++)
		{
			p_robust[i] = p_robust[i] / sum;
			if(p_robust[i] <(0.1/packet_num))
			{
				p_robust[i] = 0;
			}
			else if(p_robust[i] < temp_min)
			{
				temp_min = p_robust[i];
				min_index = i;
			}
		}	
		que_num = 1 / temp_min;
		distribution = (int*)malloc(que_num*sizeof(int));
		memset(distribution,0,que_num*sizeof(int));
		
		random_matrix_index = 0;
		j = 0;		
		for(i = 0; i < min_index;i++)
		{
			int temp_num = p_robust[i] * que_num;
			random_matrix_index += temp_num;
			//printf("i is %d",i);
			for(;j < random_matrix_index;j++)
			{
				//printf("j is %d",j);
				distribution[j] = i+1;
			}
		}
	
		//create robust solition matrix
		srand(time(NULL));				
		for(i = 0;i < matrix_distribution_len;i++)
		{
			int rand_num = rand()%random_matrix_index;
			matrix_distribution[i] = distribution[rand_num];
		}

		free(distribution);
		distribution = NULL;
		free(p_robust);
		p_robust = NULL;		
		return matrix_distribution;	
	}
}


/*fun:decode the data received */
/*entrance:row_max:the max number of row, define in platform.h
	   buf:the data received
	   current_session_num:current session num that is decoding
	   insert_index:The position of data that should be inserted in the decoding matrix
 	   matrix_message_decode:decoding matrix of message
	   matrix_H_decode:decoding matrix of H  
	   success_recover_que:when a source packet has been successed recover,it will be record into the success_recover_que 
	   que_insert_pos: The position that the success recover packet should be record  */
/*return:0 means decoding faild then wait next block
	 any value >0 means decoding success(packet_encode_num means the number of source packets is divided into small pieces) */
int receiveAndDecode(int row_max,char* buf,int* current_session_num,int* insert_index,char*** matrix_message_decode,int*** matrix_H_decode,int* success_recover_que,int* que_insert_pos)
{
	int session_num;
	int packet_encode_num;
	int i;
	memcpy(&session_num,buf,blocksize_sessionNum);
	memcpy(&packet_encode_num,buf+blocksize_sessionNum,blocksize_packetNum);
	//if there is no session,then set up the session,or the session num of the packet is not current session,then throw this package
	for (i = 0;i < MAX_SUCCESS_QUENUM;i++)
	{
		if (session_num == success_recover_que[i])
		{
			return -1;
		}
	}
	if(*current_session_num == 0)
	{
		*current_session_num = session_num;
		*insert_index = 0;
		init_receiver(row_max,packet_encode_num,blocksize_data_Len,matrix_message_decode,matrix_H_decode);
	}
	else if(session_num != *current_session_num)
	{
		return -1;
	}

	memcpy((*matrix_message_decode)[*insert_index],buf+blocksize_sessionNum+blocksize_packetNum,blocksize_data_Len);
	memcpy((*matrix_H_decode)[*insert_index],buf+blocksize_sessionNum+blocksize_packetNum+blocksize_data_Len,packet_encode_num*sizeof(int));
	*insert_index = *insert_index + 1;
	if(*insert_index > packet_encode_num)
	{
		if(*insert_index == 24)
		{
			printf("receive 24");
		}
		if(Gussian_decode(row_max,packet_encode_num,blocksize_data_Len,*matrix_message_decode,*matrix_H_decode))
		{
			success_recover_que[*que_insert_pos] = *current_session_num;
			*que_insert_pos = *que_insert_pos + 1;
			if (*que_insert_pos == MAX_SUCCESS_QUENUM)
			{
				*que_insert_pos = 0;
			}
			*current_session_num = 0;
			*insert_index = 0;						
			return packet_encode_num;
		}
	}
	if(*insert_index == row_max)
	{
		*insert_index = 0;
	}
	return 0;
}

void init_receiver(int row_max,int col_H_max,int col_message_max,char*** matrix_message_decode,int*** matrix_H_decode)
{
	int i;
	if(!(*matrix_H_decode))
	{
		*matrix_H_decode = (int**)malloc(sizeof(int*)*row_max);
	}	
	for(i = 0;i < row_max;i++)
	{
		(*matrix_H_decode)[i] =(int*)malloc(sizeof(int)*col_H_max);
		memset((*matrix_H_decode)[i],0,sizeof(int)*col_H_max);
	}
	*matrix_message_decode = (char**)malloc(sizeof(char*)*row_max);
	for(i = 0;i < row_max;i++)
	{
		(*matrix_message_decode)[i] = (char*)malloc(sizeof(char)*col_message_max);
		memset((*matrix_message_decode)[i],0,sizeof(char)*col_message_max);
	}
}

void free_receiver(int row_max,char*** matrix_message_decode,int*** matrix_H_decode)
{
	int i;	
	for(i = 0;i < row_max;i++)
	{
		free((*matrix_H_decode)[i]) ;
		(*matrix_H_decode)[i] = NULL;
	}
	free((*matrix_H_decode));
	(*matrix_H_decode) = NULL;	
	for(i = 0;i < row_max;i++)
	{
		free((*matrix_message_decode)[i]);
		(*matrix_message_decode)[i] = NULL;
	}
	free((*matrix_message_decode));
	*matrix_message_decode = NULL;
}

int Gussian_decode(int row_max,int col_H_max,int col_message_max,char** matrix_message_decode,int** matrix_H_decode)
{
	int col_index;
	int row_index;
	int H_rank;
	for(col_index = 0;col_index < col_H_max;col_index++)
	{
		//If the diagonal element is not 1,then find found a row where this col is 1, then swap
		if(matrix_H_decode[col_index][col_index] != 1)
		{
			row_index = col_index + 1;
			for(;row_index < row_max;row_index++)
			{
				if(matrix_H_decode[row_index][col_index] == 1)
				{
					row_swap_int(&matrix_H_decode[col_index],&matrix_H_decode[row_index]);
					row_swap_char(&matrix_message_decode[col_index],&matrix_message_decode[row_index]);
					break;
				}
			}
			if(row_index == row_max)
			{
				return 0;
			}
		}
		for(row_index = 0;row_index < row_max;row_index++)
		{
			if(row_index != col_index)
			{
				if(matrix_H_decode[row_index][col_index] == 1)
				{
					LT_XOR_int(matrix_H_decode[row_index],matrix_H_decode[col_index],col_H_max,matrix_H_decode[row_index]);
					LT_XOR_char(matrix_message_decode[row_index],matrix_message_decode[col_index],col_message_max,matrix_message_decode[row_index]);
				}
			}
		}

	}	
	if((H_rank = find_rank(col_H_max,matrix_H_decode)) == col_H_max)
	{
		return 1;
	}
	return 0;
}


int find_rank(int rank_max,int** matrix_H_decode)
{
	int H_rank = 0;
	int i;
	for(i = 0; i < rank_max; i++)
	{
		if(matrix_H_decode[i][i] == 1)
		{
			H_rank++;
		}
	}
	return H_rank;
}

//LT function end