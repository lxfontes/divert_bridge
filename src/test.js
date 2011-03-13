tcp_counter = 0;

function timer_handler(){
  print("tcp packets ",tcp_counter);
}
function packet_handler(pkt,iface){
  if(pkt.pkt_type == "tcp" && pkt.tcp_data.length > 0){
    tcp_counter++;
    if(pkt.tcp_data.indexOf("playboy") != -1){
      print("Blocking");
      print(pkt.tcp_data);
      divert_block(pkt);
      return;
    }
  }

  //only call this one if you know what you're doing ;)
//  divert_flip(pkt);

  //call either divert_pass or divert_block on each packet
  //default = divert_pass
  divert_pass(pkt);
}
print("script loaded");
