function [ send_packet ] = LT_link_simulate(packet_num,packet_length,decode_tag,receive_packet_statistic,packet_loss)
%LT_LINK_SIMULATE Summary of this function goes here
%   Detailed explanation goes here
        %消息矩阵
        message_matrix = randi([0 1],packet_num,packet_length);

        % %初始化接收方——高斯消元
        % H_decode = []; 
        % code_decode = []; 
        % success_tag = 1;
        % receive_packet = 0;

        %初始化接收方--BP译码
        rank_statistic = [0];
        H_decode = zeros(packet_num,packet_num);
        code_decode = zeros(packet_num,packet_length);

        receive_packet = 0;  
        send_packet = 0;

        %鲁棒孤波分布
        distribution_matrix_prob = robust_solition(packet_num);  

        success_tag = 0;
        while(~success_tag)
            %得到本次发送度数
            send_degree = randsrc(1,1,[1:size(distribution_matrix_prob,2);distribution_matrix_prob]);
            %send_degree = distribution_matix(randi(size(distribution_matix,1))); 
            
            %编码初始化
            H = zeros(1,packet_num);
            code_encode = zeros(1,packet_length);
            message_encode_pos = [];

            %随机挑选度数个原始数据包   
            i = 1;
            while i <= send_degree
                i = i + 1;
                temp_pos = randi(size(message_matrix,1));
                if H(temp_pos) == 0
                    H(temp_pos) = 1;
                    message_encode_pos = [message_encode_pos temp_pos];
                else
                    i = i-1;
                end
            end    

            %对挑选出的原始数据包进行异或编码    
            for i = 1:size(message_encode_pos,2)            
                code_encode =  rem(code_encode+message_matrix(message_encode_pos(i),:),2);
            end  

            %发送
            code_send{1,1} = code_encode; 
            code_send{1,2} = H;
            send_packet = send_packet + 1;


            %接收
            

            %计算链路丢包
            packet_loss_tag = randsrc(1,1,[[0 1];[1 - packet_loss packet_loss]]);            
            if packet_loss_tag == 1
                rank_statistic = [rank_statistic rank_statistic(size(rank_statistic,2))]; 
                continue;
            end

            if decode_tag == 1
                %BP译码
                receive_packet = receive_packet + 1; 
                [H_decode,code_decode,tag_decode] = LT_decode_BP(code_send{1,2},code_send{1,1},H_decode,code_decode);
                rank_statistic = [rank_statistic find_rank(H_decode)];
            elseif decode_tag == 2
                %高斯译码
                receive_packet = receive_packet + 1; 
                [H_decode,code_decode,tag_decode,rank_statistic] = LT_decode_Guassian(code_send{1,2},code_send{1,1},H_decode,code_decode,rank_statistic); 
            end

            if tag_decode == 1                       
                disp('decode success');
                disp('receive packet num is');
                disp(receive_packet);
                disp('send packet num is');
                disp(send_packet);
                if decode_tag == 1
                    %BP译码
                elseif decode_tag == 2
                    %高斯译码
                    receive_packet_statistic = [receive_packet_statistic receive_packet];
                end
                
                send_bit_total = receive_packet * packet_length;
                success_tag = 1;
%                 plot(rank_statistic / packet_num)  %错误率图像
                break;
            end 
        end 

end

