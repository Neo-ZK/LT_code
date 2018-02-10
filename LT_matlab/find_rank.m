function [ rank_value ] = find_rank( H )
    rank_value = 0;
    for i = 1:min(size(H))
        if H(i,i) == 1
            rank_value = rank_value + 1;
        end
    end 
end

