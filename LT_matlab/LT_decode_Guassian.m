function [ H_decode_after,code_decode_after,tag_decode ] = LT_decode_Guassian( H_receive,code_receive,H_decode_before,code_decode_before)
    tag_decode = 0;
    H_decode_after = [H_decode_before;
                      H_receive];
    code_decode_after = [code_decode_before;
                        code_receive];
                    
    [H_decode_after,code_decode_after] = Gussian(H_decode_after,code_decode_after);
    rank_H = find_rank(H_decode_after);
    if rank_H == size(H_decode_after,2)
        tag_decode = 1;
    end


end

