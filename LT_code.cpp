#include "LT_code.h"

LT_code::~LT_code()
{
    delete _filename;
    //message matrix dtor
    if(_buf)
    {
        delete []_buf;
    }
    if(_matrix_message_row)
    {
        for(int i = 0;i < _matrix_message_row;i++)
        {
            delete []_matrix_message[i];
        }
        delete []_matrix_message;
    }

    //encode matrix dtor
    if(_matrix_encode)
    {
        for(int i = 0;i < _matrix_encode_row;i++)
        {
            delete []_matrix_encode[i];
        }
        delete []_matrix_encode;
    }
    //H
    if(_H)
    {
        for(int i = 0;i < _matrix_encode_row;i++)
        {
            delete []_H[i];
        }
        delete []_H;
    }
    //robust distribution
    if(_matrix_dgree_distribution)
    {
        delete[] _matrix_dgree_distribution;
    }
    if(_matrix_H_decode)
    {

    }

}

void LT_code::LT_XOR(int* arr1,int* arr2,int len,int* result)
{
    for(int i = 0;i < len;i++)
    {
        result[i] = arr1[i] ^ arr2[i];
    }
}

void LT_code::LT_XOR(char* arr1,char* arr2,int len,char* result)
{
    for(int i = 0;i < len;i++)
    {
        result[i] = arr1[i] ^ arr2[i];
    }
}



/************************************************************************/
/* fun:get the size of file                                             */
/* Entrance:nothing
/* Return:the size of file
/************************************************************************/
int LT_code::get_filesize()
{
    struct stat f_stat;
    if (stat(_filename, &f_stat) == -1)
    {
        return -1;
    }
    return f_stat.st_size;
}

/************************************************************************/
/* fun:Read the files to be processed into the matrix                   */
/* Entrance:nothing
/* Return:nothing
/************************************************************************/
void LT_code::ReadFile()
{
    ifstream infile;
    int filesize = get_filesize();
    infile.open(_filename,ios::in|ios::binary);
    if(!infile)
    {
        cout<<"fail to open the file"<<endl;
        exit(1);
    }
    _matrix_message_row = filesize/_matrix_encode_col + 1;
    _matrix_message = new char*[_matrix_message_row];
    for(int i = 0;i < _matrix_message_row;i++)
    {
        _matrix_message[i] = new char[_matrix_encode_col];
        memset(_matrix_message[i],0,sizeof(char)*_matrix_encode_col);
        infile.read(_matrix_message[i],_matrix_encode_col);
    }
    /*
    _buf = new char[filesize];
    infile.read(_buf,filesize);
    for (int i = 0; i < filesize; i++)
	{
		printf("0x%x ", (unsigned char)*(_buf + i));
	}
	*/
    infile.close();
}

void LT_code::encodeAndsend()
{
    ReadFile();
    init_sender();
    _matrix_dgree_distribution = robust_solition(_matrix_message_row,_redundancy);
    _matrix_encode_row = _matrix_message_row * (1 + _redundancy);
    srand(time(NULL));
    _send_buf = new char[_session_num_len+_matrix_encode_row*sizeof(int)+_matrix_encode_col];
    int* temp_H = new int[_matrix_encode_row];
    char* temp_code = new char[_matrix_encode_col];
    int session_num = rand()%100;
    int test_current_session_num = 0;
    int test_insert_index = 0;
    int test_packet_encode_num = 0;
	int test_recv_packet_num = 0;
	//WriteFile("../test2.jpg",_matrix_message_row);
    for(int row_index = 0;row_index < _matrix_encode_row;row_index++)
    {
        memset(temp_H,0,_matrix_encode_row*sizeof(int));
        memset(temp_code,0,_matrix_encode_col);
        int degree_num = _matrix_dgree_distribution[row_index];
        for(int col_index = 0;col_index < degree_num;col_index++)
        {
            int packet_index = rand()%_matrix_message_row;
			if (temp_H[packet_index] == 0)
			{
				temp_H[packet_index] = 1;
				LT_XOR(temp_code,_matrix_message[packet_index],_matrix_encode_col,temp_code);
			}
			else
			{
				col_index--;
			} 
        }
        memcpy(_send_buf,&session_num,_session_num_len);
        memcpy(_send_buf+_session_num_len,&_matrix_message_row,_packet_encode_num_len);
        memcpy(_send_buf+_session_num_len+_packet_encode_num_len,temp_code,_matrix_encode_col);
        memcpy(_send_buf+_session_num_len+_packet_encode_num_len+_matrix_encode_col,temp_H,_matrix_encode_row*sizeof(int));
        char* test_send_buf = _send(_send_buf,_session_num_len+_packet_encode_num_len+_matrix_encode_col+_matrix_encode_row*sizeof(int));
        //test receive
        int row_max = 200;
		if (receiveAndDecode(row_max,test_send_buf,test_current_session_num,test_insert_index))
		{
			break;
		}
		else
		{
			test_recv_packet_num++;
		}
        
    }
    delete[] temp_code;
    delete[] temp_H;
}

/************************************************************************/
/* fun:generate degree distribution based on robust solition            */
/* Entrance:packet_num:message packet num. redundacy:encoding redundancy */
/* Return:degree distribution */
/************************************************************************/

int* LT_code::robust_solition(int packet_num,double redundancy)
{
    //ideal solition
    double* p_robust = new double[packet_num];
    memset(p_robust,0,packet_num);
    p_robust[0] = (double)1/packet_num;
    for(int i = 1;i < packet_num;i++)
    {
        p_robust[i] = (double)1/(i*(i+1));
    }
    //robust solition
    double c = 0.05;
    double delta = 0.05; //The probability of successful decoding is guaranteed as 1- delta
    double R = c * log((double)packet_num/delta) * (double)sqrt((double)packet_num);
    int degree_max = (int)(packet_num/R);

    for(int i = 0;i < degree_max;i++)
    {
        double p = R/((i+1)*packet_num);
        p_robust[i] = p_robust[i] + p;
    }
    //normalization
    double sum = 0;
    for(int i = 0;i < packet_num;i++)
    {
        sum += p_robust[i];
    }
    double temp_min = 1;
    int min_index = 0;
    for(int i = 0;i < packet_num;i++)
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
    int que_num = 1 / temp_min;
    int* distribution = new int[que_num];
    int random_matrix_index = 0;
    int j = 0;
    for(int i = 0; i < min_index;i++)
    {
        int temp_num = p_robust[i] * que_num;
        random_matrix_index += temp_num;
        for(;j < random_matrix_index;j++)
        {
            distribution[j] = i+1;
        }
    }

    int matrix_distribution_len = packet_num * (1 + redundancy);
    int* matrix_distribution = new int[matrix_distribution_len];
    srand(time(NULL));
    for(int i = 0;i < matrix_distribution_len;i++)
    {
        int rand_num = rand()%random_matrix_index;
        matrix_distribution[i] = distribution[rand_num];
    }


    delete[] distribution;
    delete[] p_robust;
    return matrix_distribution;

}


/************************************************************************/
/* fun:initial the sender                                               */
/* Entrance:nothing
/* Return:nothing
/************************************************************************/
void LT_code::init_sender()
{
    int row_num = _matrix_message_row*(1+_redundancy);
    _H = new int*[row_num];
    for(int i = 0;i < row_num;i++)
    {
        _H[i] = new int[_matrix_message_row];
        memset(_H[i],0,_matrix_message_row);
    }
    _matrix_encode = new char*[row_num];
    for(int i = 0;i < row_num;i++)
    {
        _matrix_encode[i] = new char[_matrix_encode_col];
        memset(_matrix_encode[i],0,_matrix_encode_col);
    }
}

/************************************************************************/
/* fun:initial the receiver                                             */
/* Entrance:row_max:the max row num of matrix H and matrix message
            col_H_max:the max col num of matrix H
            col_message_max:the max col num of matrix message
/* Return:nothing
/************************************************************************/
void LT_code::init_receiver(int row_max,int col_H_max,int col_message_max)
{
    _matrix_H_decode = new int*[row_max];
    for(int i = 0;i < row_max;i++)
    {
        _matrix_H_decode[i] = new int[col_H_max];
        memset(_matrix_H_decode[i],0,col_H_max);
    }
    _matrix_message_decode = new char*[row_max];
    for(int i = 0;i < row_max;i++)
    {
        _matrix_message_decode[i] = new char[col_message_max];
        memset(_matrix_message_decode[i],0,col_message_max);
    }
}

/************************************************************************/
/* fun:receive package,then decode                                      */
/* Entrance:row_max:the max row num of matrix H and matrix message
            buf: receive package
            current_session_num:current session num
            insert_index:the position of  insert
/* Return:nothing
/************************************************************************/
int LT_code::receiveAndDecode(int row_max,char* buf,int& current_session_num,int& insert_index)
{
    int session_num;
    int packet_encode_num;
    memcpy(&session_num,buf,_session_num_len);
    memcpy(&packet_encode_num,buf+_session_num_len,_packet_encode_num_len);
    //if there is no session,then set up the session,or the session num of the packet is not current session,then throw this package
    if(current_session_num == 0)
    {
        current_session_num = session_num;
        insert_index = 0;
        init_receiver(row_max,packet_encode_num,_matrix_encode_col);
    }
    else if(session_num != current_session_num)
    {
        return 0;
    }

    memcpy(_matrix_message_decode[insert_index],buf+_session_num_len+_packet_encode_num_len,_matrix_encode_col);
    memcpy(_matrix_H_decode[insert_index],buf+_session_num_len+_packet_encode_num_len+_matrix_encode_col,packet_encode_num*sizeof(int));
    insert_index++;
    if(insert_index > packet_encode_num)
    {
        if(Gussian_decode(row_max,packet_encode_num,_matrix_encode_col))
        {
            WriteFile("../test1.jpg",packet_encode_num);
			return 1;
        }
    }
    if(insert_index == row_max)
    {
        insert_index = 0;
    }

	return 0;

}

/************************************************************************/
/* fun:Gauss elimination function                                       */
/* Entrance:row_max:the max row num of matrix H and matrix message
            col_H_max:the max col num of matrix H
            col_message_max:the max col num of matrix message
/* Return:1 represent decode success,0 represent fail
/************************************************************************/
int LT_code::Gussian_decode(int row_max,int col_H_max,int col_message_max)
{
    for(int col_index = 0;col_index < col_H_max;col_index++)
    {
        //If the diagonal element is not 1,then find found a row where this col is 1, then swap
        if(_matrix_H_decode[col_index][col_index] != 1)
        {
            int row_index = col_index + 1;
            for(;row_index < row_max;row_index++)
            {
                if(_matrix_H_decode[row_index][col_index] == 1)
                {
                    swap(_matrix_H_decode[col_index],_matrix_H_decode[row_index]);
                    swap(_matrix_message_decode[col_index],_matrix_message_decode[row_index]);
					break;
                }
            }
            if(row_index == row_max)
            {
                return 0;
            }
        }       
        for(int row_index = 0;row_index < row_max;row_index++)
        {
            if(row_index != col_index)
            {
                if(_matrix_H_decode[row_index][col_index] == 1)
                {
                    LT_XOR(_matrix_H_decode[row_index],_matrix_H_decode[col_index],col_H_max,_matrix_H_decode[row_index]);
                    LT_XOR(_matrix_message_decode[row_index],_matrix_message_decode[col_index],col_message_max,_matrix_message_decode[row_index]);
                }
            }
        }
        
    }
    int H_rank;
    if((H_rank = find_rank(col_H_max)) == col_H_max)
    {
        return 1;
    }
    return 0;
}


/************************************************************************/
/* fun:find the rank of matrix                                          */
/* Entrance:the max num of col and row
/* Return:the rank of matrix
/************************************************************************/
int LT_code::find_rank(int rank_max)
{
    int H_rank = 0;
    for(int i = 0; i < rank_max; i++)
    {
        if(_matrix_H_decode[i][i] == 1)
        {
            H_rank++;
        }
    }
    return H_rank;
}

/************************************************************************/
/* fun:recover the file from decode matrix                              */
/* Entrance:nothing
/* Return:nothing
/************************************************************************/
void LT_code::WriteFile(const char* filename,int line_num)
{
    ofstream outfile;
    outfile.open(filename,ios::binary);
    for(int i = 0;i < line_num;i++)
    {
        outfile.write(_matrix_message_decode[i],_matrix_encode_col);
    }
    outfile.close();
}
