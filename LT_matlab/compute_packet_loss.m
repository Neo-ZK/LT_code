function [ packet_loss ] = compute_packet_loss( ser,length )
    packet_loss =1 - (1-ser)^length;
end

