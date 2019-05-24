function [H,code_encode] = LT_encode( source_code,redundancy )

    packet_num = size(source_code,1);
    packet_encode_num = ceil(packet_num * (1 + redundancy));
    pakcet_length = size(source_code,2);
    code_encode = zeros(packet_encode_num,pakcet_length);
    %鲁棒孤波分布
    distribution_matix = robust_solition(packet_num,redundancy);

    %H为校验矩阵
    H = zeros(packet_encode_num,packet_num);

    %生成校验矩阵
%     for col_index = 1:packet_num
%         一个包节点连接的校验节点的个数
%         pk2check_num = distribution_matix(col_index);
%         row_pos = randperm(packet_encode_num,pk2check_num);
%         for row_index = row_pos
%             H(row_index,col_index) = 1;
%         end
%     end
    
    for row_index = 1:packet_encode_num
        pk2check_num = distribution_matix(row_index);
        col_pos = randperm(packet_num,pk2check_num);
        for col_index = col_pos
             H(row_index,col_index) = 1;
             code_encode(row_index,:) = rem(code_encode(row_index,:)+source_code(col_index,:),2);
        end
    end
    
    %校验矩阵的每一行中所有为1的包模2加后为发送的包
%     for encode_row_index = 1:packet_encode_num
%         encode_pos = find(H(encode_row_index,:) == 1);
%         for source_row_index = encode_pos
%             code_encode(encode_row_index,:) = rem( code_encode(encode_row_index,:)+source_code(source_row_index,:),2);
%         end
%     end
%     
    

end

