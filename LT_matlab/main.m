%发送信息参数
packet_num  = 1000;
packet_length = 50;
redundancy = 0.2;
code_send = cell(1,2);
%消息矩阵
message_matrix = randi([0 1],packet_num,packet_length);
[H,code_encode] = LT_encode(message_matrix,redundancy);
%初始化接收方
H_decode = [];
code_decode = []; 
send_index = randperm(size(code_encode,1));
%send_index = 1:size(code_encode,1);
receive_packet = 0;
for i = send_index
    %发送
    code_send{1,1} = code_encode(i,:); 
    code_send{1,2} = H(i,:);    
    %接收译码
    
    receive_packet = receive_packet + 1;
    if receive_packet == 500
        a = 1;
    end
    [H_decode,code_decode,tag_decode] = LT_decode_Guassian(code_send{1,2},code_send{1,1},H_decode,code_decode);   
    if tag_decode == 1
        receive_pkt = size(code_decode,1);        
        disp('decode success');
        disp('receive packet num is');
        disp(receive_pkt);
        break;
    end    
end
 