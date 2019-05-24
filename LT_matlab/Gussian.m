function [ H_in,code_in ] = Gussian( H_in,code_in )
    for col_index = 1:min(size(H_in,1),size(H_in,2))
        row_pos = find(H_in(:,col_index) ~= 0);
        if(isempty(row_pos))
            return;
        end 
        if(H_in(col_index,col_index) == 0)
            %对角元不为1则找为1的行交换  
            row_pos_below = find(H_in(col_index+1:size(H_in,1),col_index) ~= 0);
            if(isempty(row_pos_below))
                return;
            end            
            row_pointer = row_pos_below(1) + col_index;                                  
            temp_H = H_in(row_pointer,:);
            H_in(row_pointer,:) = H_in(col_index,:);
            H_in(col_index,:) = temp_H;
            temp_code = code_in(row_pointer,:);
            code_in(row_pointer,:) = code_in(col_index,:);
            code_in(col_index,:) = temp_code;
            for row_index = row_pos'
                if(row_index ~= row_pointer)                
                    H_in(row_index,:) = rem(H_in(col_index,:) + H_in(row_index,:),2);
                    code_in(row_index,:)  = rem(code_in(col_index,:) + code_in(row_index,:),2);
                end
            end 
        else
            for row_index = row_pos'
                if(row_index ~= col_index)                
                    H_in(row_index,:) = rem(H_in(col_index,:) + H_in(row_index,:),2);
                    code_in(row_index,:)  = rem(code_in(col_index,:) + code_in(row_index,:),2);
                end
            end                        
        end       
    end  
end

