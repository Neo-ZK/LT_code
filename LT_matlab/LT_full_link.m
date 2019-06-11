clear;
clc;
%循环次数
loop_num = 1;
%统计数据
receive_packet_statistic = [];

%链路传输速率 
speed = 100000;
send_bit_matrix_orig_mat = zeros(1,50);send_bit_matrix_orig_mat1 = zeros(1,50);

%译码算法--2为GE，1为BP
decode_tag = 2;


%误码率
ser_matrix = 1e-4;


packet_loss_num = 0;
dynamic_statistic = cell(1,loop_num);



code_send = cell(1,2);
send_bit_total = 0;

%发送参数
file_length = 100000;  %数据总长度
K_base = 100; %码长（喷泉码码长即原始数据的分包数）

w = 3000;
%仿真次数
for loop = 1:loop_num
%     ser_receive_packet_statisitc = [];
%     K_statistic = [];
%     ser_receive_packet_statisitc1 = [];
%     ser_receive_packet_statisitc2 = [];
    for p = ser_matrix 
        %发送信息参数 
        K = K_base; %当次发送码长，LT码码长确定可以自定义一些策略，这里只是简单写死
        packet_num = K;
        packet_length = file_length/K;
%         K_statistic = [K_statistic K];%统计数据，带_statistic的均为统计数据，不影响实际传输过程
        packet_loss = compute_packet_loss( p,packet_length); %根据包长和误码率确定丢包率，也可以设定固定丢包率
        send_packet =  LT_link_simulate(packet_num,packet_length,decode_tag,receive_packet_statistic,packet_loss); 
        send_redudancy = send_packet*packet_length/file_length;
%         ser_receive_packet_statisitc = [ser_receive_packet_statisitc send_redudancy];   
    end
%     dynamic_statistic{loop} = ser_receive_packet_statisitc;   
end


